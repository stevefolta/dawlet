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
#ifdef TEST_SMALL_TESTS
	extern void TestSmallTests();
	TestSmallTests();
#endif
}

