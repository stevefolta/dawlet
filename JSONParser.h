#ifndef JSONParser_h
#define JSONParser_h

#include <string>


class JSONParser {
	public:
		JSONParser(std::string text_in)
			: text(text_in) {}

		// Objects.
		void	start_object();
		std::string	next_field();
		void	start_array();
		bool	array_is_done();
		void	ignore_value();
		int	next_int();
		double	next_double();
		std::string	next_string();
		bool	next_bool();

		/* How to use the object parsing:
			parser.start_object();
			while (true) {
				std::string field_name = parser.next_field();
				if (field_name.empty())
					break;
				if (field_name == "foo")
					//...
				else if (field_name == "bar")
					//...
				else {
					// Must be some future field of the object; ignore it.
					parser.ignore_value();
					}
				}

			parser.start_array();
			while (!parser.array_is_done()) {
				// Read the element of the array.
				}
		*/

		// Basic token reading.
		std::string	next_token();
		std::string	peek_token();

	protected:
		std::string	text;
		std::string	peeked_token;
	};


#endif	// !JSONParser_h

