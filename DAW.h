#ifndef DAW_h
#define DAW_h

#include <string>

class AudioEngine;
class AudioFileRead;
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

		void	add_file_read(AudioFileRead* read);

	protected:
		AudioEngine*	engine;
		Web::Server*	server;
		AudioFileRead*	active_reads;
		bool	running;

		bool	handle_file_reads();
		void	open_project(std::string path);
	};


#endif	// !DAW_h

