#ifndef AudioEngine_h
#define AudioEngine_h

#include "BufferManager.h"
class MessageQueue;


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

		void	send(int message);

	protected:
		int	cur_sample_rate;
		int	cur_buffer_size;

		BufferManager*	bufferManager;
		MessageQueue*	to;
		MessageQueue*	from;

		void	run();
		static void*	thread_start(void* arg);
	};

extern AudioEngine* audioEngine;


#endif 	// !AudioEngine_h
