#include "DAW.h"
#include "AudioEngine.h"
#include "web/Server.h"
#include "web/Connection.h"
#include "FieldParser.h"
#include <unistd.h>
#include <stdio.h>

using namespace std;


DAW::DAW(int server_port)
{
	engine = new AudioEngine();
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
	/***/
}


