#include "JSONParser.h"

using namespace std;


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
				//*** Throw an exception?
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



