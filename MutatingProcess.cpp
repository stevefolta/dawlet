#include "MutatingProcess.h"
#include "DAW.h"


MutatingProcess::MutatingProcess()
{
	daw->add_pending_mutation();
}


void MutatingProcess::mutation_done()
{
	daw->mutation_complete();
}



