#include "Process.h"


bool SimpleProcess::is_done()
{
	return state == Done;
}


void SimpleProcess::next()
{
	switch (state) {
		case InEngine:
			in_engine();
			state = BackInDAW;
			break;
		case BackInDAW:
			back_in_daw();
			state = Done;
			break;
		}
}



