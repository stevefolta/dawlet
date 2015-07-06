#include "../JSONParser.h"
#include "../Exception.h"
#include <stdio.h>

void TestJSONParser()
{
	static const char* text =
		"{\n"
		"	\"foo\": [ 1, \"two\", false ],\n"
		"	\"bar\": { \"baz\": 3.14159 }\n"
		"}\n";

	try {
		// Basic tokenization.
		{
		JSONParser parser(text);
		while (true) {
			std::string token = parser.next_token();
			if (token.empty())
				break;
			printf("%s\n", token.c_str());
			}
		}

		// Objects.
		{
		JSONParser parser(text);
		parser.start_object();
		while (true) {
			std::string field_name = parser.next_field();
			if (field_name.empty())
				break;
			if (field_name == "foo") {
				// This is a heterogenous array, unlike what you'd see in real
				// life.
				parser.start_array();
				while (!parser.array_is_done()) {
					// Just consume the value, which will be a single token.
					parser.ignore_value();
					}
				}
			else if (field_name == "bar") {
				parser.start_object();
				while (true) {
					field_name = parser.next_field();
					if (field_name.empty())
						break;
					parser.ignore_value();
					}
				}
			else
				throw Exception("unexpected-field-name");
			}
		printf("Object parsing passed.\n");
		}
		}
	catch (Exception& e) {
		fprintf(stderr, "%s\n", e.type.c_str());
		}
}


