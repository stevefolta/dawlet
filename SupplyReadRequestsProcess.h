#ifndef SupplyReadRequestsProcess_h
#define SupplyReadRequestsProcess_h

#include "Process.h"
class AudioFileReadRequest;


class SupplyReadRequestsProcess : public Process {
	public:
		SupplyReadRequestsProcess(int num_requests_in);
		~SupplyReadRequestsProcess();

		bool	is_done() ;
		void	next() ;

	protected:
		enum {
			Supplying,
			Done
			};
		int	state;

		enum {
			num_requests = 8,
			};

		AudioFileReadRequest*	next_request;
		AudioFileReadRequest*	requests[num_requests];

		void	supplying();
	};


#endif	// !SupplyReadRequestsProcess_h

