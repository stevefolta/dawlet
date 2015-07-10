#include "Base64.h"

// Adapted from the Wikipedia page.

static const char* codes =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";


std::string	base64_encode_to_string(const void* data_in, int length)
{
	const unsigned char* data = (const unsigned char*) data_in;

	enum {
		buffer_size = 128,
		};
	char buffer[buffer_size];
	char* out = buffer;
	char* out_stopper = buffer + buffer_size - 4;
	std::string result = "";

	const unsigned char* stopper = data + length;
	for (; data < stopper; data += 3) {
		int b = (*data & 0xFC) >> 2;
		*out++ = codes[b];
		b = (*data & 0x03) << 4;
		if (data + 1 < stopper) {
			b |= (data[1] & 0xF0) >> 4;
			*out++ = codes[b];
			b = (data[1] & 0x0F) << 2;
			if (data + 2 < stopper) {
				b |= (data[2] & 0xC0) >> 6;
				*out ++ = codes[b];
				b = data[2] & 0x3F;
				*out++ = codes[b];
				}
			else {
				*out++ = codes[b];
				*out++ = '=';
				}
			}
		else {
			*out++ = codes[b];
			*out++ = '=';
			*out++ = '=';
			}

		if (out >= out_stopper) {
			result += std::string(buffer, out);
			out = buffer;
			}
		}

	if (out > buffer)
		result += std::string(buffer, out);

	return result;
}



