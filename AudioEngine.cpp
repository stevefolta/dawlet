#include "AudioEngine.h"
#include "MessageQueue.h"
#include "Process.h"
#include "Logger.h"
#include <pthread.h>
#include <unistd.h>

AudioEngine* engine = nullptr;


AudioEngine::AudioEngine()
{
	engine = this;

	cur_sample_rate = 44100;
	cur_buffer_size = 128;

	play_head = 0.0;

	bufferManager = new BufferManager;
	to = new MessageQueue();
	from = new MessageQueue();

	for (int i = 0; i < max_read_requests; ++i)
		read_requests[i] = nullptr;

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
}


void AudioEngine::start()
{
	/***/
}


void AudioEngine::stop()
{
	/***/
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


int AudioEngine::receive_audio_file_read_requests(int num_requests, AudioFileReadRequest** requests)
{
	int next_request = 0;

	for (int i = 0; i < max_read_requests; ++i) {
		if (read_requests[i] == nullptr) {
			read_requests[i] = requests[next_request++];
			if (next_request >= num_requests)
				break;
			}
		}

	// Returns the number of requests taken.
	return next_request;
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

		// Prepare the next buffer.
		/***/

		// Send the buffer to the interface.
		/***/
		usleep(10000);
		}
}


void* AudioEngine::thread_start(void* arg)
{
	((AudioEngine*) arg)->run();
	return nullptr;
}



