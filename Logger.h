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


#endif 	// !Logger_h

