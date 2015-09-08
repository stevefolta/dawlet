#ifndef SetTrackRecordArmedProcess_h
#define SetTrackRecordArmedProcess_h

#include "MutatingProcess.h"
class Track;
namespace Web {
	class Connection;
	}


class SetTrackRecordArmedProcess : public MutatingProcess {
	public:
		SetTrackRecordArmedProcess(Track* track, bool new_armed, Web::Connection* connection);
		bool	is_done();
		void	next();

	protected:
		enum {
			Setting,
			Replying,
			Done
			};

		int	state;
		Track*	track;
		bool	new_armed;
		Web::Connection*	connection;
	};



#endif	// !SetTrackRecordArmedProcess_h

