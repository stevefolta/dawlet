#ifndef Recorder_h
#define Recorder_h

#include <vector>
class AudioBuffer;
class RecordBuffers;


class Recorder {
	public:
		Recorder();
		~Recorder();

		void	start();
		void	stop();

		void	write_buffers(RecordBuffers* record_buffers);

	protected:
		int	num_armed_channels;
		std::vector<AudioBuffer*>	capture_buffers;
	};


#endif	// !Recorder_h

