#ifndef Server_h
#define Server_h

#include <vector>

class DAW;


namespace Web {

	class Connection;

	class Server {
		public:
			Server(int port, DAW* daw);
			~Server();

			bool	tick();

		protected:
			DAW*	daw;
			int	listen_socket;
			std::vector<Connection*>	connections;
		};

	}


#endif	// !Server_h

