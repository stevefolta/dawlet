#include "Logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


Logger* logger = nullptr;


void NullLogger::log(const char* message)
{
	// Do nothing.
}


void StdoutLogger::log(const char* message)
{
	fwrite(message, strlen(message), 1, stdout);
}



void log(const char* message, ...)
{
	enum {
		max_message_size = 256,
		};

	char full_message[max_message_size];
	va_list args;
	va_start(args, message);
	int length = vsnprintf(full_message, max_message_size - 1, message, args);
	va_end(args);

	// Add a newline.
	if (length >= max_message_size - 2)
		length = max_message_size - 2;
	full_message[length] = '\n';
	full_message[length + 1] = 0;

	logger->log(full_message);
}



