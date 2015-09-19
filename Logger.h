#ifndef Logger_h
#define Logger_h

class Logger {
	public:
		virtual void	log(const char* message) = 0;
	};


class NullLogger : public Logger {
	public:
		void	log(const char* message);
	};


class StdoutLogger : public Logger {
	public:
		void	log(const char* message);
	};


extern void log(const char* message, ...);

extern Logger* logger;



// Stopwatch.
// A facility for timing things for debugging.  The start of the timed period
// is when the Stopwatch is created.  The end of the timed period can either be
// explicit (by calling stop()), or when the stopwatch falls out of scope.

#include <time.h>

class Stopwatch {
	public:
		Stopwatch(const char* msg_in, int min_reported_us_in = 0);
		~Stopwatch();

		void start(const char* new_msg, int new_min_reported_us = 0);
		void stop();

	protected:
		const char* msg;
		struct timespec start_time;
		int min_reported_us;
		bool running;
	};


#endif 	// !Logger_h

