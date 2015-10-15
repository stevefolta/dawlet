#ifndef GetProcess_h
#define GetProcess_h

#include "Process.h"
namespace Web {
	class Connection;
	}


class GetProcess : public Process {
	public:
		GetProcess(Web::Connection* connection_in)
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

		virtual void	in_engine() = 0;
		virtual void	replying() = 0;
	};


#endif	// !GetProcess_h

