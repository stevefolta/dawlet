#include "AudioEngine.h"
#include "MessageQueue.h"
#include "Process.h"
#include "AudioFileRead.h"
#include "SendMeteringProcess.h"
#include "AudioSystem.h"
#include "AudioInterface.h"
#include "Project.h"
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
	project = nullptr;

	bufferManager = new BufferManager;
	to = new MessageQueue();
	from = new MessageQueue();

	free_read_requests = nullptr;
	num_free_read_requests = 0;

	buffers_until_metering = 1;
	next_metering_process = nullptr;

	pthread_t thread;
	pthread_create(&thread, nullptr, &thread_start, this);
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
	while (true) {
		// First, handle messages from the non-realtime thread.
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

		// Make sure we have enough free AudioFileReadRequests.
		if (num_free_read_requests < 40 /* TODO: project->num_tracks() * factor */) {
			from->send(
				Message::NeedMoreReadRequests,
				40 - num_free_read_requests /* TODO */);
			}

		bufferManager->run();

		AudioInterface* interface = audio_system->selected_interface();
		if (interface == nullptr) {
			usleep(10000);
			continue;
			}

		// Prepare the next buffer.
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
		if (playing) {
			if (!missing_buffers)
				project->run(out_buffers, num_channels);
			play_head += (ProjectPosition) cur_buffer_size / cur_sample_rate;
			}

		// Send the buffers to the interface.
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

		run_metering();
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


void AudioEngine::send_next_metering_process()
{
	SendMeteringProcess* process = next_metering_process;
	next_metering_process = process->next_peaks;
	process->next_peaks = nullptr;
	return_process(process);
}



