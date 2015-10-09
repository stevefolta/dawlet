#include "APIHandler.h"
#include "web/Connection.h"


void APIHandler::handle(std::string url_remainder, Web::Connection* connection)
{
	// Default: reply with json_value.

	send_json_reply(connection, json_value());
}


std::string APIHandler::json_value()
{
	// Default: should be overridden if handle() isn't.
	return "";
}


bool APIHandler::can_put()
{
	// Default: no:
	return false;
}


void APIHandler::handle_put(std::string url_remainder, std::string value, Web::Connection* connection)
{
	// Default: doesn't handle PUT.
}


bool APIHandler::can_post()
{
	// Default: no:
	return false;
}


void APIHandler::handle_post(std::string url_remainder, Web::Connection* connection)
{
	// Default: doesn't handle POST.
}


void APIHandler::send_json_reply(Web::Connection* connection, std::string json)
{
	connection->send_json_reply(json);
}


void APIHandler::send_ok_reply(Web::Connection* connection)
{
	connection->send_ok_reply();
}



std::string pop_url_front(std::string* url_remainder)
{
	std::string front;
	int slash_pos = url_remainder->find('/');
	if (slash_pos != std::string::npos) {
		front = url_remainder->substr(0, slash_pos);
		url_remainder->erase(0, slash_pos + 1);
		}
	else {
		int question_pos = url_remainder->find('?');
		if (question_pos != std::string::npos) {
			front = url_remainder->substr(0, question_pos);
			url_remainder->erase(0, question_pos);
			}
		else {
			front = *url_remainder;
			*url_remainder = "";
			}
		}
	return front;
}



