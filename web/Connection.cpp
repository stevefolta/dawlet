#include "Connection.h"
#include "Request.h"
#include "FieldParser.h"
#include "Exception.h"
#include "Logger.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

using namespace Web;
using namespace std;


Connection::Connection(int socket_in)
	: socket(socket_in), cur_request(nullptr)
{
	buffer = new Buffer();
	send_buffer = new Buffer();
	state = StartingRequest;
}


Connection::~Connection()
{
	shutdown(socket, SHUT_RDWR);
	close(socket);
	delete send_buffer;
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
		else if (result == 0) {
			// EOF: The client closed the connection.
			state = Closed;
			}
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
		buffer->clear();
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
		if (header_name.back() != ':') {
			error_out("400 Bad Request");
			return;
			}
		header_name.pop_back();

		// Add the header.
		string value = fields.remainder();
		cur_request->add_header(header_name, value);
		log("Header: \"%s\": \"%s\".", header_name.c_str(), value.c_str());
		}
}


void Connection::handle_request()
{
	if (cur_request == nullptr) {
		// We're in a bad state, just die.
		state = Closed;
		return;
		}

	if (cur_request->type() == "GET") {
		get_file();
		}

	else
		error_out("501 Not Implemented");
}


void Connection::get_file()
{
	// Get info about the file.
	string path;
	if (cur_request->path() == "/")
		path = "html/index.html";
	else
		path = "html/" + cur_request->path();
	struct stat file_info;
	int result = stat(path.c_str(), &file_info);
	if (result == -1 || !S_ISREG(file_info.st_mode)) {
		error_out("404 Not Found");
		return;
		}
	off_t size = file_info.st_size;

	// Read the file contents.
	char* contents = (char*) malloc(size);
	bool ok = false;
	FILE* file = fopen(path.c_str(), "r");
	if (file) {
		ok = true;
		result = fread(contents, 1, size, file);
		if (result != size)
			ok = false;
		fclose(file);
		}
	if (!ok) {
		error_out("500 Internal Server Error");
		free(contents);
		return;
		}

	// Send the headers.
	send_line("HTTP/1.1 200 OK");
	send_content_length(size);
	send_line_fragment("Content-Type: ");
	send_line(content_type_for(path));
	send_line();
	flush_send_buffer();

	// Send the file contents.
	result = write(socket, contents, size);
	if (result == -1)
		throw Exception("send-fail");

	free(contents);
	state = StartingRequest;
}


void Connection::error_out(string code)
{
	send_buffer->clear();
	send_line("HTTP/1.1 " + code);
	send_content_length(code.length());
	send_line();
	send_line_fragment(code);
	send_reply();
	state = Closed;
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


void Connection::send_line(string line)
{
	// Adds the line to the send buffer.

	int length = line.length();
	if (length + 2 > buffer_size)
		throw Exception("line-overflow");
	if (send_buffer->bytes_left() < length + 2)
		flush_send_buffer();

	memcpy(&send_buffer->data[send_buffer->filled], line.data(), length);
	send_buffer->filled += length;
	send_buffer->data[send_buffer->filled++] = '\r';
	send_buffer->data[send_buffer->filled++] = '\n';
}


void Connection::send_line()
{
	if (send_buffer->bytes_left() < 2)
		flush_send_buffer();
	send_buffer->data[send_buffer->filled++] = '\r';
	send_buffer->data[send_buffer->filled++] = '\n';
}


void Connection::send_line_fragment(std::string text)
{
	// Adds the text to the send buffer.
	int length = text.length();
	if (length > buffer_size)
		throw Exception("line-overflow");
	if (send_buffer->bytes_left() < length)
		flush_send_buffer();

	memcpy(&send_buffer->data[send_buffer->filled], text.data(), length);
	send_buffer->filled += length;
}


void Connection::send_reply()
{
	// Finishes sending the reply.

	flush_send_buffer();
}


void Connection::flush_send_buffer()
{
	int result = write(socket, send_buffer->data, send_buffer->filled);
	if (result == -1)
		throw Exception("send-fail");
	send_buffer->clear();
}


void Connection::send_content_length(unsigned long length)
{
	char content_length_header[64];
	sprintf(content_length_header, "Content-Length: %lu", length);
	send_line(content_length_header);
}


std::string Connection::content_type_for(std::string filename)
{
	struct ContentType {
		const char*	suffix;
		const char*	content_type;
		};
	static const ContentType content_types[] = {
		{ "html", "text/html" },
		{ "css", "text/css" },
		{ "js", "application/javascript" },
		{ nullptr, nullptr }
		};

	size_t dot_position = filename.rfind('.');
	if (dot_position != string::npos) {
		string suffix = filename.substr(dot_position + 1);
		const ContentType* pair = content_types;
		for (; pair->suffix; ++pair) {
			if (suffix == pair->suffix)
				return pair->content_type;
			}
		}

	// Default: "application/octet-stream".
	return "application/octet-stream";
}



