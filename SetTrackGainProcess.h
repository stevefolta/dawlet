#ifndef SetTrackGainProcess_h
#define SetTrackGainProcess_h

#include "Process.h"
class Track;
namespace Web {
	class Connection;
	}


class SetTrackGainProcess : public Process {
	public:
		SetTrackGainProcess(Track* track, float gain, Web::Connection* connection);
		bool	is_done();
		void	next();

	protected:
		enum {
			Setting,
			Replying,
			Done
			};

		int state;
		Track*	track;
		float gain;
		Web::Connection*	connection;
	};


#endif	// !SetTrackGainProcess_h

