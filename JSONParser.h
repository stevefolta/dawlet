#ifndef JSONParser_h
#define JSONParser_h

#include <string>


class JSONParser {
	public:
		JSONParser(std::string text_in)
			: text(text_in) {}

		std::string	next_token();
		std::string	peek_token();

	protected:
		std::string	text;
		std::string	peeked_token;
	};


#endif	// !JSONParser_h

