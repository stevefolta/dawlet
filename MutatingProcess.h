#ifndef MutatingProcess_h
#define MutatingProcess_h

#include "Process.h"


class MutatingProcess : public Process {
	public:
		MutatingProcess();

		void	mutation_done();
	};



#endif	// !MutatingProcess_h

