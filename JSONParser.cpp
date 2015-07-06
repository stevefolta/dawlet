#include "JSONParser.h"
#include "Exception.h"

using namespace std;


void JSONParser::start_object()
{
	if (next_token() != "{")
		throw Exception("json-object-expected");
}


std::string JSONParser::next_field()
{
	string token = next_token();
	if (token == ",")
		token = next_token();
	if (token == "}")
		return "";
	if (token[0] != '"')
		throw Exception("json-field-name-expected");
	string field_name(token.begin() + 1, token.end() - 1);
	if (next_token() != ":")
		throw Exception("json-bad-field");
	return field_name;
}


void JSONParser::start_array()
{
	if (next_token() != "[")
		throw Exception("json-array-expected");
}


bool JSONParser::array_is_done()
{
	// This is a bit too lenient; it'll accept arrays that don't have their
	// elements separated by commas.
	string token = peek_token();
	if (token == ",") {
		next_token();
		token = peek_token();
		}
	if (token == "]") {
		next_token();
		return true;
		}
	return false;
}


void JSONParser::ignore_value()
{
	string token = peek_token();
	if (token == "[") {
		start_array();
		while (true) {
			ignore_value();
			if (array_is_done())
				break;
			}
		}
	else if (token == "{") {
		start_object();
		while (!next_field().empty())
			ignore_value();
		}
	else {
		// Everything else is a single-token value.
		next_token();
		}
}


std::string JSONParser::next_token()
{
	if (!peeked_token.empty()) {
		string result = peeked_token;
		peeked_token.clear();
		return result;
		}

	// Skip whitespace.
	string::iterator p = text.begin();
	char c = 0;
	for (; p < text.end(); ++p) {
		c = *p;
		if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
			break;
		}

	// Read the token, depending on what kind it is.
	// At the end, "p" will point just past the end of the token.
	string::iterator token_start = p;
	++p;
	switch (c) {
		case '"':
			// String.
			{
			bool complete = false;
			while (p < text.end()) {
				c = *p++;
				if (c == '"') {
					complete = true;
					break;
					}
				else if (c == '\\') {
					// Just skip the next character.  If it's '"', we consumed the
					// exact escape sequence.  If it's not, it may be a
					// single-character escape, in which case we've also consumed
					// the exact escape sequence, or it's just like consuming
					// characters like we're already doing.
					++p;
					}
				}
			if (!complete) {
				throw Exception("unterminated-json-string");
				token_start = p;
				}
			}
			break;

		case '{':  case '}':
		case '[':  case ']':
		case ',':
		case ':':
			// Single-character tokens.
			// We're all done, nothing left to do.
			break;

		case '0':  case '1':  case '2':  case '3':  case '4':
		case '5':  case '6':  case '7':  case '8':  case '9':
		case '-':  case '+':
			// Number.
			{
			bool got_dot = false;
			bool got_e = false;
			bool done = false;
			for (; p < text.end(); ++p) {
				switch (*p) {
					case '0':  case '1':  case '2':  case '3':  case '4':
					case '5':  case '6':  case '7':  case '8':  case '9':
						break;
					case '.':
						if (got_dot)
							done = true;
						got_dot = true;
						break;
					case 'e':  case 'E':
						if (got_e)
							done = true;
						got_e = true;
						break;
					default:
						done = true;
						break;
					}
				if (done)
					break;
				}
			}
			break;

		case 0:
		case ' ':  case '\t':  case '\n':  case '\r':
			// EOF.
			token_start = p = text.end();
			break;

		default:
			// Anything else is just an alphabetic token, and we hope it'll be
			// "true", "false", or "null".
			for (; p < text.end(); ++p) {
				c = *p;
				if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
					// Keep going.
					}
				else
					break;
				}
			break;
		}

	string result(token_start, p);
	text.erase(text.begin(), p);
	return result;
}


std::string JSONParser::peek_token()
{
	if (peeked_token.empty())
		peeked_token = next_token();
	return peeked_token;
}



