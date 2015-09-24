#ifndef Recorder_h
#define Recorder_h

#include <vector>
#include <map>
#include <string>
class AudioBuffer;
class RecordBuffers;
class Track;
namespace Web {
	class Connection;
	}


class Recorder {
	public:
		Recorder();
		~Recorder();

		void	arm_track(Track* track, Web::Connection* reply_connection = nullptr);
		void	unarm_track(Track* track, Web::Connection* reply_connection = nullptr);
		void	set_track_input(Track* track, std::string input, Web::Connection* reply_connection = nullptr);

		void	start();
		void	stop();

		void	interface_changed();
		void	project_changed();

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

