#ifndef Request_h
#define Request_h

#include <string>
#include <map>


namespace Web {

	class Request {
		public:
			Request(std::string type_in, std::string path_in)
				: the_type(type_in), the_path(path_in) {}

			void	add_header(std::string name, std::string value)
				{ headers[name] = value; }
			std::string	header(std::string name)
				{ return headers[name]; }

			std::string	type() { return the_type; }
			std::string	path() { return the_path; }
			std::string	content() { return the_content; }

			void	append_content(std::string more_content)
				{ the_content += more_content; }

		protected:
			std::string	the_type;
			std::string	the_path;
			std::string	the_content;
			std::map<std::string, std::string>	headers;
		};

	}



#endif	// !Request_h

