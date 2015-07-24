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


void APIHandler::send_json_reply(Web::Connection* connection, std::string json)
{
	connection->send_line("HTTP/1.1 200 OK");
	connection->send_content_length(json.length());
	connection->send_line("Content-Type: application/json");
	connection->send_line();
	connection->send_line_fragment(json);
	connection->flush_send_buffer();
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
		front = *url_remainder;
		*url_remainder = "";
		}
	return front;
}



