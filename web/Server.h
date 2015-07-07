#ifndef Server_h
#define Server_h

#include <vector>


namespace Web {

	class Connection;

	class Server {
		public:
			Server(int port);
			~Server();

			bool	tick();

		protected:
			int	listen_socket;
			std::vector<Connection*>	connections;
		};

	}


#endif	// !Server_h

