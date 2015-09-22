#include "GetPBHeadProcess.h"
#include "AudioEngine.h"
#include "web/Connection.h"
#include <stdio.h>


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



