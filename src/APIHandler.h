#ifndef APIHandler_h
#define APIHandler_h

#include <string>
namespace Web {
	class Connection;
	};


class APIHandler {
	public:
		virtual void	handle(std::string url_remainder, Web::Connection* connection);
		virtual std::string	json_value();

		virtual bool	can_put();
		virtual void	handle_put(std::string url_remainder, std::string value, Web::Connection* connection);

		virtual bool	can_post();
		virtual void	handle_post(std::string url_remainder, Web::Connection* connection);

		virtual bool	can_delete();
		virtual void	handle_delete(std::string url_remainder, Web::Connection* connection);

		void	send_json_reply(Web::Connection* connection, std::string json);
		void	send_ok_reply(Web::Connection* connection);

	protected:
	};


extern std::string pop_url_front(std::string* url_remainder);


#endif	// !APIHandler_h

