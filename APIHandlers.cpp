#include "APIHandlers.h"
#include "web/Connection.h"
#include <map>


void APIHandler_project::handle(std::string url_remainder, Web::Connection* connection)
{
	connection->error_out("501 Not Implemented");
	/***/
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

	std::string api;
	int slash_pos = url_remainder.find('/');
	if (slash_pos != std::string::npos) {
		api = url_remainder.substr(0, slash_pos);
		url_remainder = url_remainder.substr(slash_pos + 1);
		}
	else {
		api = url_remainder;
		url_remainder = "";
		}
	APIHandler* handler = handlers_map[api];
	if (handler)
		handler->handle(url_remainder, connection);
}


