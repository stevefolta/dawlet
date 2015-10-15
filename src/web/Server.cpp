#include "Server.h"
#include "Connection.h"
#include "DAW.h"
#include "Exception.h"
#include "Logger.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

using namespace Web;


Server::Server(int port, DAW* daw_in)
	: daw(daw_in)
{
	// Set up the listen socket.
	listen_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == -1)
		throw Exception("socket-open-fail");
	int yes = 1;
	int result =
		setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	result = bind(listen_socket, (struct sockaddr*) &addr, sizeof(addr));
	if (result == -1)
		throw Exception("socket-bind-fail");
	result = listen(listen_socket, 10);
	if (result == -1)
		throw Exception("socket-listen-fail");
}


Server::~Server()
{
	while (!connections.empty()) {
		delete connections.back();
		connections.pop_back();
		}

	if (listen_socket != -1)
		close(listen_socket);
}


bool Server::tick()
{
	bool did_something = false;
	if (listen_socket == -1)
		return false;

	// Listen for a new connection.
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(listen_socket, &fds);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	int result = select(listen_socket + 1, &fds, NULL, NULL, &timeout);
	if (result == -1)
		throw Exception("select-fail");
	if (result > 0) {
		int connection_socket = accept(listen_socket, NULL, NULL);
		if (connection_socket == -1)
			throw Exception("accept-fail");
		connections.push_back(new Connection(connection_socket, daw));
		}

	// Handle connections.
	for (auto it = connections.begin(); it != connections.end(); ++it) {
		Connection* connection = *it;
		if (connection->is_closed()) {
			connections.erase(it);
			delete connection;
			// The iterator went bad, we'll just have to get to the rest of the
			// connections on the next tick.
			break;
			}
		else
			did_something |=  connection->tick();
		}

	return did_something;
}



