#ifndef SupplyReadsProcess_h
#define SupplyReadsProcess_h

#include "Process.h"
class AudioFileRead;
class DAW;


class SupplyReadsProcess : public Process {
	public:
		SupplyReadsProcess(DAW* daw_in, int num_requests_in);
		~SupplyReadsProcess();

		bool	is_done() ;
		void	next() ;

	protected:
		DAW*	daw;

		enum {
			Supplying,
			Done
			};
		int	state;

		AudioFileRead*	next_request;

		void	supplying();
	};


#endif	// !SupplyReadsProcess_h

