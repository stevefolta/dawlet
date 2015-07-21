#include "AudioEngine.h"
#include "MessageQueue.h"
#include "Process.h"
#include "AudioFileRead.h"
#include "AudioSystem.h"
#include "AudioInterface.h"
#include "Project.h"
#include "Exception.h"
#include "Logger.h"
#include <pthread.h>
#include <unistd.h>

AudioEngine* engine = nullptr;


AudioEngine::AudioEngine()
{
	engine = this;

	cur_sample_rate = 44100;
	cur_buffer_size = 128;

	play_start = play_head = 0.0;
	playing = false;
	project = nullptr;

	bufferManager = new BufferManager;
	to = new MessageQueue();
	from = new MessageQueue();

	free_read_requests = nullptr;
	num_free_read_requests = 0;

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

				case Message::ContinueProcess:
					{
						Process* process = (Process*) message->param;
						process->next();
						from->send(Message::ContinueProcess, process);
						}
					break;

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
		AudioBuffer* out_buffer = get_buffer();
		if (out_buffer) {
			out_buffer->clear();
			if (playing) {
				project->run(out_buffer);
				play_head += (ProjectPosition) cur_buffer_size / cur_sample_rate;
				}
			}

		// Send the buffer to the interface.
		if (out_buffer) {
			try {
				interface->wait_until_ready();
				interface->send_data(out_buffer->samples);
				}
			catch (Exception& e) {
				fprintf(stderr, "Exception during audio send: %s.\n", e.type.c_str());
				}
			free_buffer(out_buffer);
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
}


void* AudioEngine::thread_start(void* arg)
{
	((AudioEngine*) arg)->run();
	return nullptr;
}



