#include "APIHandlers.h"
#include "web/Connection.h"
#include "Project.h"
#include "DAW.h"
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
	connection->error_out("501 Not Implemented");
	/***/
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


