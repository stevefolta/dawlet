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

		protected:
			enum {
				buffer_size = 1024,
				compaction_point = buffer_size / 2,
				};

			enum {
				StartingRequest,
				ReadingHeaders,
				ReportingError,
				};

			struct Buffer {
				int	read, filled;
				char	data[buffer_size];

				Buffer()
					: read(0), filled(0) {}
				};

			int	socket;
			int	state;
			Buffer*	buffer;
			Request*	cur_request;

			void	process_buffer();
			void	compact_buffer();
			void	start_request();
			void	read_headers();
			void	handle_request();
			void	error_out(std::string code);

			struct LineResult {
				bool	ok;
				std::string	line;
				};

			LineResult	next_line();
		};

	}


#endif	// !Connection_h

