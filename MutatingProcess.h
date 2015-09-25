#ifndef MutatingProcess_h
#define MutatingProcess_h

#include "Process.h"


class MutatingProcess : public Process {
	public:
		MutatingProcess(bool really_mutating = true);

		void	mutation_done();

	protected:
		bool	really_mutating;
	};



#endif	// !MutatingProcess_h

