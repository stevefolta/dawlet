#include "APIHandlers.h"
#include "web/Connection.h"
#include "Project.h"
#include "Track.h"
#include "DAW.h"
#include "AudioEngine.h"
#include "SetTrackStateProcesses.h"
#include "Logger.h"
#include <map>


std::string APIHandler_project::json_value()
{
	Project* project = daw->cur_project();
	if (project)
		return project->api_json();
	else
		return "null";
}


void APIHandler_track::handle(std::string url_remainder, Web::Connection* connection)
{
	// Get the track.
	std::string id_str = pop_url_front(&url_remainder);
	if (id_str.empty()) {
		connection->error_out("404 Not Found");
		return;
		}
	int id = strtol(id_str.c_str(), nullptr, 0);
	Track* track = nullptr;
	Project* project = daw->cur_project();
	if (id > 0 && project)
		track = project->track_by_id(id);
	if (track == nullptr) {
		connection->error_out("404 Not Found");
		return;
		}

	if (url_remainder.empty())
		send_json_reply(connection, track->api_json());
	else if (url_remainder == "clips")
		send_json_reply(connection, track->clips_json());
	else {
		connection->error_out("404 Not Found");
		}
}


bool APIHandler_track::can_put()
{
	return true;
}


void APIHandler_track::handle_put(std::string url_remainder, std::string value, Web::Connection* connection)
{
	// Get the track.
	std::string id_str = pop_url_front(&url_remainder);
	if (id_str.empty()) {
		connection->error_out("404 Not Found");
		return;
		}
	int id = strtol(id_str.c_str(), nullptr, 0);
	Track* track = nullptr;
	Project* project = daw->cur_project();
	if (id > 0 && project)
		track = project->track_by_id(id);
	if (track == nullptr) {
		connection->error_out("404 Not Found");
		return;
		}

	if (url_remainder.empty())
		connection->error_out("405 Method Not Allowed");
	else {
		std::string component = pop_url_front(&url_remainder);
		if (!url_remainder.empty()) {
			connection->error_out("404 Not Found");
			return;
			}
		if (component == "gain") {
			char* end_ptr;
			float gain = strtof(value.c_str(), &end_ptr);
			if (end_ptr == value.c_str())
				connection->error_out("400 Bad Request");
			else
				engine->start_process(new SetTrackGainProcess(track, gain, connection));
			}
		else if (component == "name")
			engine->start_process(new SetTrackNameProcess(track, value, connection));
		else if (component == "record-arm") {
			bool armed = false, ok = true;
			if (value == "true")
				armed = true;
			else if (value != "false") {
				connection->error_out("400 Bad Request");
				ok = false;
				}
			if (ok) {
				engine->start_process(
					new SetTrackRecordArmedProcess(track, armed, connection));
				}
			}
		else
			connection->error_out("404 Not Found");
		}
}


void APIHandler_stats::handle(std::string url_remainder, Web::Connection* connection)
{
	connection->error_out("501 Not Implemented");
	/***/
}



#undef Handler
#define Handler(name) 	\
	APIHandler_##name	name##_handler;
#include "TopAPIHandlers.h"

struct HandlerAssoc {
	const char*	name;
	APIHandler*	handler;
	};
static HandlerAssoc top_level_handlers[] = {
	#undef Handler
	#define Stringify1(s)	#s
	#define Stringify(s)	Stringify1(s)
	#define Handler(name) 	\
		{ Stringify(name), &name##_handler },
	#include "TopAPIHandlers.h"
	{ nullptr, nullptr }
	};
static std::map<std::string, APIHandler*> handlers_map;
bool handlers_map_initialized = false;

static void initialize_handlers_map()
{
	if (handlers_map_initialized)
		return;

	HandlerAssoc* assoc = top_level_handlers;
	for (; assoc->name; ++assoc)
		handlers_map[assoc->name] = assoc->handler;
	handlers_map_initialized = true;
}

void dispatch_top_level_api(std::string url_remainder, Web::Connection* connection)
{
	initialize_handlers_map();

	std::string api = pop_url_front(&url_remainder);
	APIHandler* handler = handlers_map[api];
	if (handler)
		handler->handle(url_remainder, connection);
	else
		connection->error_out("404 Not Found");
}


void dispatch_top_level_api_put(
	std::string url_remainder, std::string content, Web::Connection* connection)
{
	initialize_handlers_map();

	std::string api = pop_url_front(&url_remainder);
	APIHandler* handler = handlers_map[api];
	if (handler) {
		if (handler->can_put())
			handler->handle_put(url_remainder, content, connection);
		else
			connection->error_out("405 Method Not Allowed");
		}
	else
		connection->error_out("404 Not Found");
}



