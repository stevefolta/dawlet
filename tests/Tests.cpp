#include "Tests.h"


void RunTests()
{
#ifdef TEST_JSON_PARSER
	extern void TestJSONParser();
	TestJSONParser();
#endif
#ifdef TEST_READ_PROJECT
	extern void TestReadProject();
	TestReadProject();
#endif
}

