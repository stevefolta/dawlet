#include "DAW.h"
#include "AudioEngine.h"
#include "AudioFileRead.h"
#include "web/Server.h"
#include "web/Connection.h"
#include "FieldParser.h"
#include "Message.h"
#include "GetPBHeadProcess.h"
#include "SupplyReadsProcess.h"
#include <unistd.h>
#include <stdio.h>

using namespace std;


DAW::DAW(int server_port)
	: active_reads(nullptr)
{
	engine = new AudioEngine();

	// Give the audio engine some of what it needs.
	engine->start_process(new SupplyReadsProcess(this, 8));

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
	bool have_messages = true;
	while (have_messages) {
		Message message = engine->next_message_from();
		switch (message.type) {
			case Message::None:
				have_messages = false;
				break;
			case Message::ContinueProcess:
				{
				Process* process = (Process*) message.param;
				process->next();
				if (process->is_done())
					delete process;
				}
				break;
			case Message::NeedMoreReadRequests:
				engine->start_process(new SupplyReadsProcess(this, message.num));
				break;
			}
		if (have_messages)
			did_something = true;
		}

	// Read-ahead for audio files.
	did_something |= handle_file_reads();

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


void DAW::add_file_read(AudioFileRead* read)
{
	read->next_read = active_reads;
	active_reads = read;
}


bool DAW::handle_file_reads()
{
	bool did_something = false;

	AudioFileRead** last_link = &active_reads;
	while (*last_link) {
		AudioFileRead* read = *last_link;
		if (read->read_is_complete()) {
			// Unlink it from the list.
			*last_link = read->next_read;
			did_something = true;
			}
		else {
			// Go to the next read in the list.
			last_link = &read->next_read;
			}
		}

	return did_something;
}



