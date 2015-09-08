#ifndef SetTrackStateProcesses_h
#define SetTrackStateProcesses_h

#include "MutatingProcess.h"
#include "Track.h"
namespace Web {
	class Connection;
	}


class SetTrackStateProcess : public MutatingProcess {
	public:
		SetTrackStateProcess(Track* track, Web::Connection* connection);
		bool	is_done();
		void	next();

	protected:
		enum {
			Setting,
			Replying,
			Done
			};

		virtual void	set() = 0;

		int	state;
		Track*	track;
		Web::Connection*	connection;
	};


class SetTrackRecordArmedProcess : public SetTrackStateProcess {
	public:
		SetTrackRecordArmedProcess(
			Track* track_in, bool new_armed_in, Web::Connection* connection_in)
			: SetTrackStateProcess(track_in, connection_in), new_armed(new_armed_in)
			{}

	protected:
		bool	new_armed;

		void	set() { track->record_armed = new_armed; }
	};


#endif	// !SetTrackStateProcesses_h

