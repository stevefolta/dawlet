#include "SelectInterfaceProcess.h"
#include "AudioSystem.h"


bool SelectInterfaceProcess::is_done()
{
	return state == Done;
}


void SelectInterfaceProcess::next()
{
	switch (state) {
		case Selecting:
			audio_system->select_interface(name);
			state = Done;
			break;
		}
}



