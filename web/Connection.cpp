#include "Connection.h"
#include "Request.h"
#include "FieldParser.h"
#include "Exception.h"
#include "Logger.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

using namespace Web;
using namespace std;


Connection::Connection(int socket_in)
	: socket(socket_in), cur_request(nullptr)
{
	buffer = new Buffer();
	state = StartingRequest;
}


Connection::~Connection()
{
	shutdown(socket, SHUT_RDWR);
	close(socket);
	delete buffer;
	delete cur_request;
}


bool Connection::tick()
{
	bool did_something = false;

	// Listen for new data.
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(socket, &fds);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	int result = select(socket + 1, &fds, NULL, NULL, &timeout);
	if (result == -1)
		throw Exception("select-fail");
	if (result > 0) {
		result = read(socket, buffer->data, buffer_size - buffer->filled);
		if (result == -1)
			throw Exception("read-fail");
		buffer->filled += result;
		process_buffer();
		compact_buffer();
		did_something = true;
		}

	return did_something;
}


void Connection::process_buffer()
{
	switch (state) {
		case StartingRequest:
			start_request();
			break;
		case ReadingHeaders:
			read_headers();
			break;
		}
}


void Connection::compact_buffer()
{
	if (buffer->read >= buffer->filled) {
		buffer->read = buffer->filled = 0;
		return;
		}

	int bytes_left_in_buffer = buffer_size - buffer->read;
	if (bytes_left_in_buffer > compaction_point)
		return;

	int bytes_remaining = buffer->filled - buffer->read;
	memmove(
		buffer->data,
		&buffer->data[buffer->read],
		bytes_remaining);
	buffer->read = 0;
	buffer->filled = bytes_remaining;
}


void Connection::start_request()
{
	LineResult result = next_line();
	if (!result.ok)
		return;

	FieldParser fields(result.line);
	string type = fields.next_field();
	string path = fields.next_field();
	string http_version = fields.next_field();
	cur_request = new Request(type, path);
	log(
		"Request: \"%s\" \"%s\" \"%s\".",
		type.c_str(), path.c_str(), http_version.c_str());

	// Continue by reading the headers.
	state = ReadingHeaders;
	read_headers();
}


void Connection::read_headers()
{
	while (true) {
		LineResult line_result = next_line();
		if (!line_result.ok)
			break;
		string line = line_result.line;

		// End of headers?
		if (line.empty()) {
			// Headers are done.
			handle_request();
			break;
			}

		// Header name.
		FieldParser fields(line);
		string header_name = fields.next_field();
		if (header_name.back() != ':')
			error_out("400 Bad Request");
		header_name.pop_back();

		// Add the header.
		string value = fields.remainder();
		cur_request->add_header(header_name, value);
		log("Header: \"%s\": \"%s\".", header_name.c_str(), value.c_str());
		}
}


void Connection::handle_request()
{
	/***/
}


void Connection::error_out(string code)
{
	/***/
	state = ReportingError;
}


Connection::LineResult Connection::next_line()
{
	const char* p = &buffer->data[buffer->read];
	const char* line_start = p;
	const char* stopper = &buffer->data[buffer->filled];
	LineResult result;
	result.ok = false;
	while (p < stopper) {
		char c = *p++;
		if (c == '\r' && p < stopper && *p == '\n') {
			const char* line_end = p - 1;
			++p;
			buffer->read = p - buffer->data;
			result.ok = true;
			result.line = string(line_start, line_end);
			break;
			}
		}
	return result;
}



