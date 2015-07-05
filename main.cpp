#include "AudioEngine.h"
#include "Logger.h"
#include "tests/Tests.h"
#include <string.h>


int main(int argc, char* argv[])
{
	logger = new NullLogger();

	for (int which_arg = 1; which_arg < argc; ++which_arg) {
		const char* arg = argv[which_arg];

		if (strcmp(arg, "-v") == 0) {
			delete logger;
			logger = new StdoutLogger();
			}
		}

	RunTests();

	AudioEngine engine;
	return 0;
}

