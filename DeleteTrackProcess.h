#ifndef DeleteTrackProcess_h
#define DeleteTrackProcess_h

#include "MutatingProcess.h"

class Track;
namespace Web {
	class Connection;
	}


class DeleteTrackProcess : public MutatingProcess {
	public:
		DeleteTrackProcess(Track* track_in, Web::Connection* connection_in)
			: track(track_in), connection(connection_in) {}

		bool	is_done();
		void	next();

	protected:
		enum {
			InEngine,
			BackInDAW,
			Done
			};
		int state;

		Track* track;
		Web::Connection* connection;

		void	in_engine();
		void	back_in_daw();
	};



#endif	// !DeleteTrackProcess_h

