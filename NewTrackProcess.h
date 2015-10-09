#ifndef NewTrackProcess_h
#define NewTrackProcess_h

#include "MutatingProcess.h"

class Track;
namespace Web {
	class Connection;
	}


class NewTrackProcess : public MutatingProcess {
	public:
		NewTrackProcess(Track* after_track, Web::Connection* connection);

		bool	is_done();
		void	next();

	protected:
		enum {
			InEngine,
			BackInDAW,
			Done
			};
		int	state;

		Track*	track;
		Track*	parent;
		Track*	after_track;
		Web::Connection*	connection;

		void	in_engine();
		void	back_in_daw();
	};


#endif	// !NewTrackProcess_h

