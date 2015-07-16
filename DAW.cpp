#include "DAW.h"
#include "AudioEngine.h"
#include "web/Server.h"
#include "web/Connection.h"
#include "FieldParser.h"
#include "Message.h"
#include "GetPBHeadProcess.h"
#include "SupplyReadRequestsProcess.h"
#include <unistd.h>
#include <stdio.h>

using namespace std;


DAW::DAW(int server_port)
{
	engine = new AudioEngine();

	// Give the audio engine some of what it needs.
	engine->start_process(new SupplyReadRequestsProcess());

	// Start up the webserver.
	server = new Web::Server(server_port, this);
	printf("Listening on http://localhost:%d/\n", server_port);
}


DAW::~DAW()
{
	delete server;
	delete engine;
}


void DAW::run()
{
	running = true;
	while (running) {
		bool did_something = tick();
		if (!did_something)
			usleep(10000);
		}
}


bool DAW::tick()
{
	// This is the heart of the non-realtime thread.
	// The AudioEngine handles all the realtime audio processing, and is
	// responsible for all changes to the project (once it has been loaded
	// initially).  Everything else (and in particular, anything that causes
	// memory to be allocated) happens here.

	// Handle web requests.
	bool did_something = server->tick();

	// Handle messages from the engine.
	while (true) {
		Message message = engine->next_message_from();
		if (message.type == Message::None)
			break;
		else if (message.type == Message::ContinueProcess) {
			Process* process = (Process*) message.param;
			process->next();
			if (process->is_done())
				delete process;
			}
		}

	// Read-ahead for audio files.
	//***

	return did_something;
}


void DAW::handle_ui_message(std::string message, Web::Connection* connection)
{
	FieldParser fields(message);
	string command = fields.next_field();
	if (command == "ping")
		connection->send_websocket_message("pong");
	else if (command == "get-play-head") {
		engine->send(Message::ContinueProcess, new GetPBHeadProcess(connection));
		}
	/***/
}



