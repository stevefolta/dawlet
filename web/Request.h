#ifndef Request_h
#define Request_h

#include <string>
#include <map>


namespace Web {

	class Request {
		public:
			Request(std::string type_in, std::string path_in)
				: type(type_in), path(path_in) {}

			void	add_header(std::string name, std::string value)
				{ headers[name] = value; }

		protected:
			std::string	type;
			std::string	path;
			std::map<std::string, std::string>	headers;
		};

	}



#endif	// !Request_h

