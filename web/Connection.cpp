#include "Connection.h"
#include "Exception.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

using namespace Web;


Connection::Connection(int socket_in)
	: socket(socket_in)
{
}


Connection::~Connection()
{
	shutdown(socket, SHUT_RDWR);
	close(socket);
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
		char* buffer = (char*) malloc(buffer_size);
		result = read(socket, buffer, buffer_size);
		if (result == -1) {
			free(buffer);
			throw Exception("read-fail");
			}
		process_buffer(buffer, result);
		did_something = true;
		}

	return did_something;
}


void Connection::process_buffer(char* buffer, int size)
{
	/***/
	free(buffer);
}



