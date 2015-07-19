#ifndef AudioEngine_h
#define AudioEngine_h

#include "BufferManager.h"
#include "Message.h"
class MessageQueue;
class AudioFileRead;
class Process;
class Project;


class AudioEngine {
	public:
		AudioEngine();
		~AudioEngine();

		int	sample_rate() { return cur_sample_rate; }
		int	buffer_size() { return cur_buffer_size; }

		void	start();
		void	stop();

		void	tick();

		AudioBuffer*	get_buffer()
			{ return bufferManager->get_buffer(); }
		void	free_buffer(AudioBuffer* buffer)
			{ bufferManager->free_buffer(buffer); }

		void	send(int message, void* param = nullptr);
		void	start_process(Process* process)
			{ send(Message::ContinueProcess, process); }
		void	continue_process(Process* process)
			{ send(Message::ContinueProcess, process); }
		Message	next_message_from();
		void	return_process(Process* process);

		Project*	install_project(Project* new_project);

		double	play_start, play_head;

		void	receive_audio_file_read(AudioFileRead* read_request);
		AudioFileRead*	next_audio_file_read();

		enum {
			read_ahead_seconds = 2,
			};

	protected:
		int	cur_sample_rate;
		int	cur_buffer_size;
		bool	playing;

		BufferManager*	bufferManager;
		MessageQueue*	to;
		MessageQueue*	from;

		AudioFileRead*	free_read_requests;
		int	num_free_read_requests;

		Project*	project;

		void	run();
		static void*	thread_start(void* arg);
	};

extern AudioEngine* engine;


#endif 	// !AudioEngine_h

