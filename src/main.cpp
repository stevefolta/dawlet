#include "DAW.h"
#include "Logger.h"
#include "Exception.h"
#include "tests/Tests.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char* argv[])
{
	try {
		logger = new NullLogger();

		int port = 8080;

		for (int which_arg = 1; which_arg < argc; ++which_arg) {
			const char* arg = argv[which_arg];

			if (strcmp(arg, "-v") == 0) {
				delete logger;
				logger = new StdoutLogger();
				}
			else if (strcmp(arg, "-p") == 0) {
				if (++which_arg >= argc) {
					fprintf(stderr, "Missing port argument.\n");
					return 1;
					}
				arg = argv[which_arg];
				port = strtol(arg, NULL, 0);
				if (port <= 0) {
					fprintf(stderr, "Bad port.\n");
					return 1;
					}
				}
			}

		RunTests();

		DAW daw(port);
		daw.run();
		}
	catch (Exception e) {
		fprintf(stderr, "Exception: %s.\n", e.type.c_str());
		return 1;
		}

	return 0;
}

