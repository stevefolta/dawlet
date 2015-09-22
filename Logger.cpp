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



Stopwatch::Stopwatch(const char* msg_in, int min_reported_us_in)
	: msg(msg_in), min_reported_us(min_reported_us_in), running(true)
{
	clock_gettime(CLOCK_MONOTONIC, &start_time);
}


Stopwatch::Stopwatch()
	: running(false)
{
}


Stopwatch::~Stopwatch()
{
	if (running)
		stop();
}

void Stopwatch::start(const char* new_msg, int new_min_reported_us)
{
	msg = new_msg;
	min_reported_us = new_min_reported_us;
	running = true;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
}


void Stopwatch::stop()
{
	if (!running)
		return;

	struct timespec end_time;
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	int us = (end_time.tv_nsec - start_time.tv_nsec) / 1000 + (end_time.tv_sec - start_time.tv_sec) * 1000000;
	if (us > min_reported_us)
		log("%dus for %s.", us, msg);
	running = false;
}

