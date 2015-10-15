#ifndef GetPBHeadProcess_h
#define GetPBHeadProcess_h

#include "GetProcess.h"


class GetPBHeadProcess : public GetProcess {
	public:
		GetPBHeadProcess(Web::Connection* connection_in)
			: GetProcess(connection_in) {}

	protected:
		double	position;

		void	in_engine();
		void	replying();
	};


#endif	// !GetPBHeadProcess_h

