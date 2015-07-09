#include "FieldParser.h"


std::string FieldParser::next_field()
{
	const char* p = line.data();
	const char* stopper = p + line.length();
	char c;

	// Skip whitespace.
	for (; p < stopper; ++p) {
		c = *p;
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			break;
		}

	if (p >= stopper) {
		// Nothing left.
		line = "";
		return line;
		}

	bool quoted = false;
	if (*p == '"') {
		++p;
		quoted = true;
		}

	// Find the end of the field.
	const char* field_start = p;
	const char* field_end = stopper;
	if (quoted) {
		while (p < stopper) {
			c = *p++;
			if (c == '"') {
				field_end = p - 1;
				break;
				}
			else if (c == '\\')
				++p;
			}
		}
	else {
		while (p < stopper) {
			c = *p++;
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				field_end = p - 1;
				break;
				}
			}
		}

	std::string result(field_start, field_end - field_start);
	line.erase(0, field_end - line.data());
	return result;
}


std::string FieldParser::remainder()
{
	// Skip whitespace.
	auto p = line.begin();
	auto stopper = line.end();
	for (; p < stopper; ++p) {
		char c = *p;
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			break;
		}

	return std::string(p, stopper);
}



