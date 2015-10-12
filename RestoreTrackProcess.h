#ifndef RestoreTrackProcess_h
#define RestoreTrackProcess_h

#include "MutatingProcess.h"

class Track;
namespace Web {
	class Connection;
	}


class RestoreTrackProcess: public MutatingProcess {
	public:
		RestoreTrackProcess(
			Track* track_in, Track* parent_in, Track* before_track_in,
			Web::Connection* connection_in)
			: track(track_in), parent(parent_in), before_track(before_track_in),
			  connection(connection_in), state(InEngine)
			{}

		bool	is_done();
		void	next();

	protected:
		enum {
			InEngine,
			BackInDAW,
			Done
			};
		int state;

		Track*	track;
		Track*	parent;
		Track*	before_track;
		Web::Connection* connection;

		void	in_engine();
		void	back_in_daw();
	};


#endif	// !RestoreTrackProcess_h

