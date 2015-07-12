#ifndef DAW_h
#define DAW_h

#include <string>

class AudioEngine;
namespace Web {
	class Server;
	class Connection;
	}


class DAW {
	public:
		DAW(int server_port);
		~DAW();

		void	run();
		bool	tick();

		void	handle_ui_message(std::string message, Web::Connection* connection);

	protected:
		AudioEngine*	engine;
		Web::Server*	server;
		bool	running;
	};


#endif	// !DAW_h

