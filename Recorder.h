#ifndef Recorder_h
#define Recorder_h

#include <vector>
#include <map>
class AudioBuffer;
class RecordBuffers;
class Track;


class Recorder {
	public:
		Recorder();
		~Recorder();

		void	arm_track(Track* track);
		void	unarm_track(Track* track);

		void	start();
		void	stop();

		void	interface_changed();

		void	write_buffers(RecordBuffers* record_buffers);

	protected:
		struct ArmedTrack {
			ArmedTrack(Track* track);
			~ArmedTrack();

			Track*	track;
			std::vector<int>*	capture_channels;
			//... file, etc...

			void	update_capture_channels();
			};

		std::map<int, ArmedTrack>	armed_tracks;
		std::vector<AudioBuffer*>	capture_buffers;
	};


#endif	// !Recorder_h

