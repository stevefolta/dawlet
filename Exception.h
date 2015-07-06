#ifndef Exception_h
#define Exception_h

#include <string>


class Exception {
	public:
		Exception(std::string type_in)
			: type(type_in) {}

		std::string	type;
	};


#endif	// !Exception_h

