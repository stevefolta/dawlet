#ifndef Connection_h
#define Connection_h


namespace Web {

	class Connection {
		public:
			Connection(int socket_in);
			~Connection();

			bool	tick();

		protected:
			enum {
				buffer_size = 1024,
				};

			int	socket;

			void	process_buffer(char* buffer, int size);
		};

	}


#endif	// !Connection_h

