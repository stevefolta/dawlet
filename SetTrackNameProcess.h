#ifndef SetTrackNameProcess_h
#define SetTrackNameProcess_h

#include "Process.h"
#include <string>
class Track;
namespace Web {
	class Connection;
	}


class SetTrackNameProcess : public Process {
	public:
		SetTrackNameProcess(Track* track, std::string name, Web::Connection* connection);
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
		std::string	name;
		Web::Connection*	connection;
	};



#endif	// !SetTrackNameProcess_h

