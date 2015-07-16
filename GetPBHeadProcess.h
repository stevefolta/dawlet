#ifndef GetPBHeadProcess_h
#define GetPBHeadProcess_h

#include "Process.h"
namespace Web {
	class Connection;
	}


class GetPBHeadProcess : public Process {
	public:
		GetPBHeadProcess(Web::Connection* connection_in)
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

		void	in_engine();
		double	position;
		void	replying();
	};


#endif	// !GetPBHeadProcess_h

