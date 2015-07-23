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

	protected:
	};


#endif	// !APIHandler_h

