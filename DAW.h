#ifndef DAW_h
#define DAW_h

#include <string>

class AudioEngine;
class AudioFileRead;
class Project;
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
		void	add_pending_mutation() { pending_mutations += 1; }
		void	mutation_complete();

		Project*	cur_project() { return project; }

		Web::Connection* websocket_connection() { return cur_websocket_connection; }
		void	set_websocket_connection(Web::Connection* connection)
			{ cur_websocket_connection = connection; }

	protected:
		AudioEngine*	engine;
		Web::Server*	server;
		Web::Connection*	cur_websocket_connection;
		Project*	project;
		AudioFileRead*	active_reads;
		bool	running;
		std::string	project_path;
		int	pending_mutations;

		bool	handle_messages_from_engine();
		bool	handle_file_reads();
		void	open_project(std::string path);
		void	save_project();

		void	supply_metering(int num_tracks);
	};


extern DAW* daw;


#endif	// !DAW_h

