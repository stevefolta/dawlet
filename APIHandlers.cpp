#include "APIHandlers.h"
#include "web/Connection.h"
#include "Project.h"
#include "Track.h"
#include "DAW.h"
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
	else {
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

void dispatch_top_level_api(std::string url_remainder, Web::Connection* connection)
{
	if (!handlers_map_initialized) {
		HandlerAssoc* assoc = top_level_handlers;
		for (; assoc->name; ++assoc)
			handlers_map[assoc->name] = assoc->handler;
		handlers_map_initialized = true;
		}

	std::string api = pop_url_front(&url_remainder);
	APIHandler* handler = handlers_map[api];
	if (handler)
		handler->handle(url_remainder, connection);
	else
		connection->error_out("404 Not Found");
}


