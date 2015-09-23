#ifndef Recorder_h
#define Recorder_h

class AudioBuffer;


class Recorder {
	public:
		Recorder();
		~Recorder();

		void	start();
		void	stop();

		void	write_buffer(int capture_channel, AudioBuffer* buffer);

	protected:
		int num_armed_channels;
	};


#endif	// !Recorder_h

