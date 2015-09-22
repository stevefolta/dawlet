#include "GetProcess.h"


bool GetProcess::is_done()
{
	return state == Done;
}


void GetProcess::next()
{
	switch (state) {
		case InEngine:	in_engine();	break;
		case Replying:	replying();	break;
		}
}



