#ifndef AudioEngine_h
#define AudioEngine_h

#include "BufferManager.h"
#include "Message.h"
#include "AudioTypes.h"
class MessageQueue;
class AudioFileRead;
class SendMeteringProcess;
class Process;
class Project;


class AudioEngine {
	public:
		AudioEngine();
		~AudioEngine();

		int	sample_rate() { return cur_sample_rate; }
		int	buffer_size() { return cur_buffer_size; }

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
		void	seek(ProjectPosition position);

		ProjectPosition	play_start, play_head;

		void	receive_audio_file_read(AudioFileRead* read_request);
		AudioFileRead*	next_audio_file_read();

		void	add_metering_process(SendMeteringProcess* metering_process);
		void	add_peak(int track_id, AudioSample peak);

		enum {
			read_ahead_seconds = 2,
			};

	protected:
		int	cur_sample_rate;
		int	cur_buffer_size;
		int	metering_hz;
		bool	playing;

		BufferManager*	bufferManager;
		MessageQueue*	to;
		MessageQueue*	from;

		AudioFileRead*	free_read_requests;
		int	num_free_read_requests;

		SendMeteringProcess*	next_metering_process;

		Project*	project;

		void	run();
		void	play();
		void	stop();
		void	pause();
		void	rewind();
		static void*	thread_start(void* arg);

		unsigned long buffers_until_metering;
		void	run_metering();
		void	send_next_metering_process();
	};

extern AudioEngine* engine;


#endif 	// !AudioEngine_h

