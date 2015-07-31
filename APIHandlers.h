#ifndef APIHandlers_h
#define APIHandlers_h

#include "APIHandler.h"

class APIHandler_project : public APIHandler {
	public:
		std::string	json_value();
	};

class APIHandler_track : public APIHandler {
	public:
		void	handle(std::string url_remainder, Web::Connection* connection);
		bool	can_put();
		void	handle_put(std::string url_remainder, std::string value, Web::Connection* connection);
	};

class APIHandler_stats : public APIHandler {
	public:
		void	handle(std::string url_remainder, Web::Connection* connection);
	};

void	dispatch_top_level_api(std::string url_remainder, Web::Connection* connection);
void	dispatch_top_level_api_put(
	std::string url_remainder, std::string content, Web::Connection* connection);


#endif	// !APIHandlers_h

