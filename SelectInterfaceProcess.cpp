#include "SelectInterfaceProcess.h"
#include "AudioSystem.h"
#include "web/Connection.h"
#include "DAW.h"
#include <sstream>


bool SelectInterfaceProcess::is_done()
{
	return state == Done;
}


void SelectInterfaceProcess::next()
{
	switch (state) {
		case Selecting:
			audio_system->select_interface(name);
			state = Replying;
			break;
		case Replying:
			reply();
			state = Done;
			break;
		}
}


void SelectInterfaceProcess::reply()
{
	Web::Connection* connection = daw->websocket_connection();
	if (connection == nullptr)
		return;
	std::stringstream message;
	message << "interface-selected \"" << name << "\"";
	connection->send_websocket_message(message.str());
}



