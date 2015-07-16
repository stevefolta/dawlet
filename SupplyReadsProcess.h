#ifndef SupplyReadsProcess_h
#define SupplyReadsProcess_h

#include "Process.h"
class AudioFileRead;


class SupplyReadsProcess : public Process {
	public:
		SupplyReadsProcess(int num_requests_in);
		~SupplyReadsProcess();

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

		AudioFileRead*	next_request;

		void	supplying();
	};


#endif	// !SupplyReadsProcess_h

