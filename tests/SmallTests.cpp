#include "Base64.h"
#include "sha1.h"
#include <stdio.h>


void TestSmallTests()
{
	// SHA1 & Base64.
	// Using examples from RFC 6455.
	std::string value = "dGhlIHNhbXBsZSBub25jZQ==";
	value += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	SHA1 sha1;
	sha1.addBytes(value.data(), value.length());
	unsigned char* digest = sha1.getDigest();
	static int digest_length = 20;
	std::string accept_value = base64_encode_to_string(digest, digest_length);
	free(digest);
	if (accept_value == "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=")
		printf("SHA1/Base64 succeeded.\n");
	else
		fprintf(stderr, "SHA1/Base64 failed!\n");
}

