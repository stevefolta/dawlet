#ifndef Connection_h
#define Connection_h

#include <string>

class DAW;


namespace Web {

	class Request;

	class Connection {
		public:
			Connection(int socket_in, DAW* daw);
			~Connection();

			bool	tick();
			bool	is_closed() { return state == Closed; }

			void	send_line(std::string line);
			void	send_line();
			void	send_line_fragment(std::string text);
			void	send_reply();
			void	flush_send_buffer();
			void	send_data(const void* data, size_t length);
			void	send_content_length(unsigned long length);
			std::string	content_type_for(std::string filename);
			void	error_out(std::string code);
			void	finish_reply();

			void	start_replying();
			void	send_json_reply(std::string json);
			void	send_ok_reply();

			void	send_websocket_message(std::string message, int opcode = WS_Text);

		protected:
			enum {
				buffer_size = 1024,
				compaction_point = buffer_size / 2,
				};

			enum {
				StartingRequest,
				ReadingHeaders,
				AwaitingWebSocketFrame,
				ReadingWebSocketData,
				ReadingRequestContent,
				Replying,
				Closed,
				};

			// WebSocket opcodes.
			enum {
				WS_Continuation = 0x0,
				WS_Text = 0x1,
				WS_Binary = 0x2,
				WS_CloseConnection = 0x8,
				WS_Ping = 0x9,
				WS_Pong = 0xA,
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
				int	readable_bytes()
					{ return filled - read; }
				};

			DAW*	daw;
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
			void	handle_api_get();
			void	handle_api_put();

			void	start_websocket();
			void	process_websocket_frame();
			void	read_websocket_data();
			void	send_websocket_control_reply(int opcode);

			char	masking_key[4];
			int	mask_phase;
			bool	frame_is_final, frame_is_masked;
			char	frame_opcode;
			uint64_t	frame_length_remaining;
			std::string	frame_data;
			std::string	control_frame_data;

			void	read_request_content();
			long	request_content_left_to_read;

			struct LineResult {
				bool	ok;
				std::string	line;
				};

			LineResult	next_line();
		};

	}


#endif	// !Connection_h

