#ifndef AudioEngine_h
#define AudioEngine_h

#include "BufferManager.h"
#include "Message.h"
class MessageQueue;
class AudioFileReadRequest;
class Process;


class AudioEngine {
	public:
		AudioEngine();
		~AudioEngine();

		int	sample_rate() { return cur_sample_rate; }
		int	buffer_size() { return cur_buffer_size; }

		void	start();
		void	stop();

		AudioBuffer*	get_buffer()
			{ return bufferManager->get_buffer(); }
		void	free_buffer(AudioBuffer* buffer)
			{ bufferManager->free_buffer(buffer); }

		void	send(int message, void* param = nullptr);
		void	start_process(Process* process)
			{ send(Message::ContinueProcess, process); }
		Message	next_message_from();
		void	return_process(Process* process);

		double	play_head;

		int	receive_audio_file_read_requests(int num_requests, AudioFileReadRequest** requests);

	protected:
		int	cur_sample_rate;
		int	cur_buffer_size;

		BufferManager*	bufferManager;
		MessageQueue*	to;
		MessageQueue*	from;

		enum {
			max_read_requests = 40,
			};
		AudioFileReadRequest*	read_requests[max_read_requests];

		void	run();
		static void*	thread_start(void* arg);
	};

extern AudioEngine* engine;


#endif 	// !AudioEngine_h

