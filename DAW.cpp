#include "DAW.h"
#include "AudioEngine.h"
#include "AudioFileRead.h"
#include "Project.h"
#include "ProjectReader.h"
#include "web/Server.h"
#include "web/Connection.h"
#include "FieldParser.h"
#include "Message.h"
#include "GetPBHeadProcess.h"
#include "SupplyReadsProcess.h"
#include "InstallProjectProcess.h"
#include "SelectInterfaceProcess.h"
#include "ALSAAudioSystem.h"
#include "Exception.h"
#include <unistd.h>
#include <stdio.h>
#include <sstream>

using namespace std;

DAW* daw = nullptr;


DAW::DAW(int server_port)
	: active_reads(nullptr), project(nullptr)
{
	daw = this;

	audio_system = new ALSAAudioSystem();

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

	engine->tick();

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
		engine->start_process(new GetPBHeadProcess(connection));
		}
	else if (command == "open-project") {
		string path = fields.next_field();
		open_project(path);
		}
	else if (command == "list-interfaces") {
		vector<string> interfaces = audio_system->list_interfaces();
		stringstream reply;
		reply << "interfaces [";
		bool did_one = false;
		for (auto it = interfaces.begin(); it != interfaces.end(); ++it) {
			if (did_one)
				reply << ", ";
			else
				did_one = true;
			reply << '"' << *it << '"';
			}
		reply << "]";
		connection->send_websocket_message(reply.str());
		}
	else if (command == "select-interface")
		engine->start_process(new SelectInterfaceProcess(fields.next_field()));
	else if (command == "play")
		engine->send(Message::Play);
	else if (command == "stop")
		engine->send(Message::Stop);
	else if (command == "pause")
		engine->send(Message::Pause);
	else if (command == "rewind")
		engine->send(Message::Rewind);
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
			read->next_read = nullptr;
			did_something = true;
			}
		else {
			// Go to the next read in the list.
			last_link = &read->next_read;
			}
		}

	return did_something;
}


void DAW::open_project(std::string path)
{
	// Read the file into "text".
	FILE* file = fopen(path.c_str(), "r");
	if (file == NULL) {
		/***/
		fprintf(stderr, "Couldn't open %s!\n", path.c_str());
		return;
		}
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* contents = (char*) malloc(size);
	fread(contents, size, 1, file);
	fclose(file);
	std::string text(contents, contents + size);

	// Read the project.
	Project* project = new Project(path);
	try {
		ProjectReader reader(text, project);
		project->read_json(&reader);
		project->load_audio_file_info();
		engine->start_process(new InstallProjectProcess(project));
		this->project = project;
		}
	catch (Exception& e) {
		/***/
		fprintf(stderr, "Reading project failed: %s.\n", e.type.c_str());
		delete project;
		}

	// Clean up.
	free(contents);
}



