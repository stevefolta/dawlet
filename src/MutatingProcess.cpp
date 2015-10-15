#include "MutatingProcess.h"
#include "DAW.h"


MutatingProcess::MutatingProcess(bool really_mutating_in)
	: really_mutating(really_mutating_in)
{
	if (really_mutating)
		daw->add_pending_mutation();
}


void MutatingProcess::mutation_done()
{
	if (really_mutating)
		daw->mutation_complete();
}



