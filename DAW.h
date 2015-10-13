#ifndef DAW_h
#define DAW_h

#include <string>

class AudioEngine;
class AudioFileRead;
class Project;
class Recorder;
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
		void	remove_file_read(AudioFileRead* read);
		void	add_pending_mutation() { pending_mutations += 1; }
		void	mutation_complete();

		Project*	cur_project() { return project; }
		Recorder*	get_recorder() { return recorder; }

		void	send_websocket_message(std::string message);
		Web::Connection* websocket_connection() { return cur_websocket_connection; }
		void	set_websocket_connection(Web::Connection* connection)
			{ cur_websocket_connection = connection; }

		void	interface_changed();

	protected:
		AudioEngine*	engine;
		Web::Server*	server;
		Recorder*	recorder;
		Web::Connection*	cur_websocket_connection;
		Project*	project;
		AudioFileRead*	active_reads;
		bool	running;
		int	pending_mutations;

		bool	handle_messages_from_engine();
		bool	handle_file_reads();
		void	open_project(std::string path);
		void	new_project(std::string name);
		void	save_project();
		void	save_project(Project* project);

		void	supply_metering(int num_tracks);
	};


extern DAW* daw;
extern const char* projects_dir;


#endif	// !DAW_h

