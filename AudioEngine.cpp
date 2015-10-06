#include "AudioEngine.h"
#include "MessageQueue.h"
#include "Process.h"
#include "AudioFileRead.h"
#include "RecordBuffers.h"
#include "RecordingClip.h"
#include "SendMeteringProcess.h"
#include "AudioSystem.h"
#include "AudioInterface.h"
#include "Project.h"
#include "Track.h"
#include "Clip.h"
#include "Exception.h"
#include "Logger.h"
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#ifdef USE_LOCAL_H
	#include "local.h"
#endif

AudioEngine* engine = nullptr;


AudioEngine::AudioEngine()
{
	engine = this;

	cur_sample_rate = 44100;
	cur_buffer_size = 128;
	#ifdef DEFAULT_BUFFER_SIZE
		cur_buffer_size = DEFAULT_BUFFER_SIZE;
	#endif
	metering_hz = 20;
	#ifdef DEFAULT_METERING_HZ
		metering_hz = DEFAULT_METERING_HZ;
	#endif

	play_start = play_head = 0.0;
	playing = false;
	recording = false;
	project = nullptr;

	bufferManager = new BufferManager;
	to = new MessageQueue();
	from = new MessageQueue();

	free_read_requests = nullptr;
	num_free_read_requests = 0;

	buffers_until_metering = 1;
	next_metering_process = nullptr;

	capture_buffers = nullptr;
	num_capture_buffers = 0;
	free_record_buffers = nullptr;
	recording_clips = nullptr;

	stats.reset();

	pthread_attr_t thread_attributes;
	int err = pthread_attr_init(&thread_attributes);
	err = pthread_attr_setschedpolicy(&thread_attributes, SCHED_FIFO);
	int max_priority = sched_get_priority_max(SCHED_FIFO);
	struct sched_param param;
	param.sched_priority = max_priority;
	err = pthread_attr_setschedparam(&thread_attributes, &param);
	err = pthread_attr_setinheritsched(&thread_attributes, PTHREAD_EXPLICIT_SCHED);
	pthread_t thread;
	pthread_create(&thread, &thread_attributes, &thread_start, this);
	err = pthread_attr_destroy(&thread_attributes);
}


AudioEngine::~AudioEngine()
{
	// Kill the engine.
	send(Message::EngineKill);
	// Wait for it to die.
	while (true) {
		if (from->is_empty()) {
			usleep(1000);
			continue;
			}

		Message* message = from->front();
		if (message->type == Message::EngineDied)
			break;
		from->pop();
		}

	for (int i = 0; i < num_capture_buffers; ++i)
		free_buffer(capture_buffers[i]);
	delete capture_buffers;
	delete recording_clips;

	delete to;
	delete from;
	delete bufferManager;
	delete project;
}


void AudioEngine::tick()
{
	bufferManager->tick();
}


void AudioEngine::send(int message, void* param)
{
	to->send(message, param);
}


Message AudioEngine::next_message_from()
{
	Message result;

	if (from->is_empty())
		result.type = Message::None;
	else {
		result = *from->front();
		from->pop();
		}

	return result;
}


void AudioEngine::return_process(Process* process)
{
	from->send(Message::ContinueProcess, process);
}


void AudioEngine::report_error(std::string error)
{
	from->send(Message::Error, new std::string(error));
}


Project* AudioEngine::install_project(Project* new_project)
{
	playing = false;
	Project* old_project = project;
	project = new_project;
	play_head = 0.0;
	project->prepare_to_play();
	return old_project;
}


void AudioEngine::receive_audio_file_read(AudioFileRead* read_request)
{
	read_request->next_read = free_read_requests;
	free_read_requests = read_request;
	num_free_read_requests += 1;
}


AudioFileRead* AudioEngine::next_audio_file_read()
{
	AudioFileRead* result = free_read_requests;
	if (result == nullptr)
		return result;
	free_read_requests = result->next_read;
	result->next_read = nullptr;
	num_free_read_requests -= 1;
	return result;
}


void AudioEngine::run()
{
	#ifdef TIME_ENGINE
	int32_t max_us = 500000 * (int64_t) cur_buffer_size / cur_sample_rate;
	Stopwatch timer;
	#endif

	while (true) {
		// First, handle messages from the non-realtime thread.
		#ifdef TIME_ENGINE
		timer.start("message handling", max_us);
		#endif
		while (!to->is_empty()) {
			Message* message = to->front();

			try {
				switch (message->type) {

					case Message::EngineKill:
						log("Engine dying.");
						from->send(Message::EngineDied);
						return;
						break;

					case Message::Play:
						play();
						break;
					case Message::Stop:
						stop();
						break;
					case Message::Pause:
						pause();
						break;
					case Message::StopPlay:
						if (playing)
							stop();
						else
							play();
						break;
					case Message::PausePlay:
						if (playing)
							pause();
						else
							play();
						break;
					case Message::Rewind:
						rewind();
						break;

					case Message::ContinueProcess:
						{
							Process* process = (Process*) message->param;
							process->next();
							if (process->return_immediately())
								from->send(Message::ContinueProcess, process);
						}
						break;

					}
				}
			catch (Exception& e) {
				log("Exception in engine: \"%s\".", e.type.c_str());
				}

			to->pop();
			}
		#ifdef TIME_ENGINE
		timer.stop();
		#endif

		#ifdef TIME_ENGINE
		timer.start("read requests/buffers", max_us);
		#endif

		// Make sure we have enough free AudioFileReadRequests.
		if (num_free_read_requests < 40 /* TODO: project->num_tracks() * factor */) {
			from->send(
				Message::NeedMoreReadRequests,
				40 - num_free_read_requests /* TODO */);
			}

		bufferManager->run();

		#ifdef TIME_ENGINE
		timer.stop();
		#endif

		AudioInterface* interface = audio_system->selected_interface();
		if (interface == nullptr) {
			usleep(10000);
			continue;
			}

		// Capture the next buffer.
		#ifdef TIME_ENGINE
		timer.start("capture", 2 * max_us + 200);
		#endif
		int num_capture_channels = interface->get_num_capture_channels();
		if (num_capture_channels > 0) {
			if (num_capture_channels > num_capture_buffers) {
				// (Re-)allocate the buffers.
				AudioBuffer** new_capture_buffers =
					new AudioBuffer*[num_capture_channels];
				int which_buffer = 0;
				for (; which_buffer < num_capture_buffers; ++which_buffer)
					new_capture_buffers[which_buffer] = capture_buffers[which_buffer];
				for (; which_buffer < num_capture_channels; ++which_buffer)
					new_capture_buffers[which_buffer] = get_buffer();
				delete capture_buffers;
				capture_buffers = new_capture_buffers;
				num_capture_buffers = num_capture_channels;
				}

			if (interface->capture_is_ready())
				interface->capture_data(capture_buffers, num_capture_buffers);
			else {
				for (int i = 0; i < num_capture_buffers; ++i)
					capture_buffers[i]->clear();
				}
			}
		#ifdef TIME_ENGINE
		timer.stop();
		#endif

		// Prepare the next buffer.
		#ifdef TIME_ENGINE
		timer.start("run", max_us);
		#endif
		int num_channels = 2;
		AudioBuffer* out_buffers[num_channels];
		bool missing_buffers = false;
		int which_channel;
		for (which_channel = 0; which_channel < num_channels; ++which_channel) {
			AudioBuffer* buffer = get_buffer();
			if (buffer) {
				buffer->clear();
				out_buffers[which_channel] = buffer;
				}
			else
				missing_buffers = true;
			}
		if (!missing_buffers)
			project->run(out_buffers, num_channels);
		if (playing)
			play_head += (ProjectPosition) cur_buffer_size / cur_sample_rate;
		#ifdef TIME_ENGINE
		timer.stop();
		#endif

		// Send the buffers to the interface.
		#ifdef TIME_ENGINE
		timer.start("send", max_us);
		#endif
		if (!missing_buffers) {
			try {
				interface->wait_until_ready();
				interface->send_data(out_buffers, num_channels);
				}
			catch (Exception& e) {
				fprintf(stderr, "Exception during audio send: %s.\n", e.type.c_str());
				}
			for (which_channel = 0; which_channel < num_channels; ++which_channel)
				free_buffer(out_buffers[which_channel]);
			}
		#ifdef TIME_ENGINE
		timer.stop();
		#endif

		// Metering.
		#ifdef TIME_ENGINE
		timer.start("metering", max_us);
		#endif
		run_metering();
		#ifdef TIME_ENGINE
		timer.stop();
		#endif

		// Recording.
		if (recording) {
			#ifdef TIME_ENGINE
			timer.start("recording", max_us);
			#endif

			// Send the record buffers up to the DAW thread to be written to disk.
			RecordBuffers* record_buffers = free_record_buffers;
			if (record_buffers) {
				free_record_buffers = record_buffers->next_free;
				for (int capture_channel = 0; capture_channel < num_capture_channels; ++capture_channel) {
					if (interface->channel_is_armed(capture_channel)) {
						record_buffers->add(capture_channel, capture_buffers[capture_channel]);
						capture_buffers[capture_channel] = get_buffer();
						}
					}
				record_buffers->start_write();
				}
			else {
				log("Record-buffers exhausted.");
				stats.exhausted_record_buffers += 1;
				}

			// Update the clips.
			if (recording_clips) {
				for (auto& clip: *recording_clips)
					clip.clip->length_in_frames += cur_buffer_size;
				}

			#ifdef TIME_ENGINE
			timer.stop();
			#endif
			}
		}
}


void AudioEngine::play()
{
	playing = true;
}


void AudioEngine::stop()
{
	playing = false;
	play_head = play_start;

	if (recording) {
		from->send(Message::RecordingStopped);
		dispose_record_buffers();
		}
	recording = false;

	if (project)
		project->prepare_to_play();
}


void AudioEngine::pause()
{
	playing = false;
	play_start = play_head;
	if (project)
		project->prepare_to_play();
}


void AudioEngine::rewind()
{
	playing = false;
	play_start = play_head = 0;
	if (project)
		project->prepare_to_play();
}


void AudioEngine::seek(ProjectPosition position)
{
	if (position < 0)
		position = 0;
	playing = false;
	play_start = play_head = position;
	if (project)
		project->prepare_to_play();
}


void* AudioEngine::thread_start(void* arg)
{
	((AudioEngine*) arg)->run();
	return nullptr;
}


void AudioEngine::run_metering()
{
	// Is it time?
	if (--buffers_until_metering > 0)
		return;
	float buffers_per_second = (float) cur_sample_rate / (float) cur_buffer_size;
	if (buffers_per_second < metering_hz)
		buffers_until_metering = 1;
	else
		buffers_until_metering = (int) roundf(buffers_per_second / metering_hz);

	if (project) {
		// Send the metering.
		project->run_metering();
		if (next_metering_process && !next_metering_process->is_empty())
			send_next_metering_process();

		// Get some more processes.
		from->send(
			Message::NeedMoreMetering,
			project->total_num_tracks());
		}

}


void AudioEngine::add_metering_process(SendMeteringProcess* metering_process)
{
	metering_process->next_peaks = next_metering_process;
	next_metering_process = metering_process;
}


void AudioEngine::add_peak(int track_id, AudioSample peak)
{
	if (next_metering_process == nullptr)
		return;

	next_metering_process->add_peak(track_id, peak);
	if (next_metering_process->is_full())
		send_next_metering_process();
}


void AudioEngine::got_xrun()
{
	from->send(Message::Xrun);
	stats.playback_xruns += 1;
}


void AudioEngine::got_capture_xrun()
{
	from->send(Message::Xrun);
	stats.capture_xruns += 1;
}


AudioBuffer* AudioEngine::get_capture_buffer(int which_capture_channel)
{
	if (which_capture_channel > num_capture_buffers)
		return nullptr;
	return capture_buffers[which_capture_channel];
}


void AudioEngine::start_recording(std::vector<RecordingClip>* recording_clips_in)
{
	delete recording_clips;
	recording_clips = recording_clips_in;

	// Install the clips.
	for (auto& clip: *recording_clips) {
		clip.clip->start = play_head;
		clip.track->add_clip(clip.clip);
		}

	playing = true;
	recording = true;
}


void AudioEngine::add_free_record_buffers(RecordBuffers* record_buffers)
{
	record_buffers->next_free = free_record_buffers;
	free_record_buffers = record_buffers;
}


void AudioEngine::dispose_record_buffers()
{
	while (free_record_buffers) {
		RecordBuffers* next = free_record_buffers->next_free;
		free_record_buffers->dispose();
		free_record_buffers = next;
		}
}


void AudioEngine::send_next_metering_process()
{
	SendMeteringProcess* process = next_metering_process;
	next_metering_process = process->next_peaks;
	process->next_peaks = nullptr;
	return_process(process);
}



