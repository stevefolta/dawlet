#ifndef GetStatsProcess_h
#define GetStatsProcess_h

#include "Process.h"
#include "Stats.h"
namespace Web {
	class Connection;
	}


class GetStatsProcess : public Process {
	public:
		GetStatsProcess(Web::Connection* connection_in)
			: connection(connection_in), state(InEngine) {}

		bool	is_done();
		void	next();

	protected:
		enum {
			InEngine,
			Replying,
			Done,
			};
		int state;

		Web::Connection*	connection;
		Stats	stats;

		void	in_engine();
		void	replying();
	};


#endif	// !GetStatsProcess_h

