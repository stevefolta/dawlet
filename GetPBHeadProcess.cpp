#include "GetPBHeadProcess.h"
#include "AudioEngine.h"
#include "web/Connection.h"
#include <stdio.h>


bool GetPBHeadProcess::is_done()
{
	return state == Done;
}


void GetPBHeadProcess::next()
{
	switch (state) {
		case InEngine:	in_engine();	break;
		case Replying:	replying();	break;
		}
}


void GetPBHeadProcess::in_engine()
{
	position = engine->play_head;
	state = Replying;
}


void GetPBHeadProcess::replying()
{
	char msg[64];
	sprintf(msg, "play-head %.20g", position);
	connection->send_websocket_message(msg);
	state = Done;
}



