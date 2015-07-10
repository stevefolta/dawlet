#ifndef Connection_h
#define Connection_h

#include <string>


namespace Web {

	class Request;

	class Connection {
		public:
			Connection(int socket_in);
			~Connection();

			bool	tick();
			bool	is_closed() { return state == Closed; }

		protected:
			enum {
				buffer_size = 1024,
				compaction_point = buffer_size / 2,
				};

			enum {
				StartingRequest,
				ReadingHeaders,
				Closed,
				};

			struct Buffer {
				int	read, filled;
				char	data[buffer_size];

				Buffer()
					: read(0), filled(0) {}

				void	clear()
					{ read = filled = 0; }
				int	bytes_left()
					{ return buffer_size - filled; }
				};

			int	socket;
			int	state;
			Buffer*	buffer;
			Request*	cur_request;
			Buffer*	send_buffer;

			void	process_buffer();
			void	compact_buffer();
			void	start_request();
			void	read_headers();
			void	handle_request();
			void	get_file();
			void	error_out(std::string code);

			struct LineResult {
				bool	ok;
				std::string	line;
				};

			LineResult	next_line();

			void	send_line(std::string line);
			void	send_line();
			void	send_line_fragment(std::string text);
			void	send_reply();
			void	flush_send_buffer();
			void	send_content_length(unsigned long length);
			std::string	content_type_for(std::string filename);
		};

	}


#endif	// !Connection_h

