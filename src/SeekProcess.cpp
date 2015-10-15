#include "SeekProcess.h"


bool SeekProcess::is_done()
{
	return state == Done;
}


void SeekProcess::next()
{
	switch (state) {
		case InEngine:
			engine->seek(position);
			state = Done;
			break;
		}
}



