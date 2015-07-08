#ifndef FieldParser_h
#define FieldParser_h

#include <string>


class FieldParser {
	public:
		FieldParser(std::string line_in)
			: line(line_in) {}

		std::string	next_field();

	protected:
		std::string	line;
	};


#endif	// !FieldParser_h

