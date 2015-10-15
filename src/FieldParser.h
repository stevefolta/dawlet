#ifndef FieldParser_h
#define FieldParser_h

#include <string>


class FieldParser {
	public:
		FieldParser(std::string line_in)
			: line(line_in) {}

		std::string	next_field();
		std::string	remainder();

	protected:
		std::string	line;
	};


#endif	// !FieldParser_h

