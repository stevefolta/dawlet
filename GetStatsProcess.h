#ifndef GetStatsProcess_h
#define GetStatsProcess_h

#include "GetProcess.h"
#include "Stats.h"


class GetStatsProcess : public GetProcess {
	public:
		GetStatsProcess(Web::Connection* connection_in)
			: GetProcess(connection_in) {}

	protected:
		Stats	stats;

		void	in_engine();
		void	replying();
	};


#endif	// !GetStatsProcess_h

