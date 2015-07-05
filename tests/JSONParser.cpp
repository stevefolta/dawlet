#include "../JSONParser.h"
#include <stdio.h>

void TestJSONParser()
{
	static const char* text =
		"{\n"
		"	\"foo\": [ 1, \"two\", false],\n"
		"	\"bar\": { \"baz\": 3.14159 }\n"
		"}\n";

	JSONParser parser(text);
	while (true) {
		std::string token = parser.next_token();
		if (token.empty())
			break;
		printf("%s\n", token.c_str());
		}
}


