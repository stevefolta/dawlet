#include "Date.h"
#include <time.h>


std::string compact_iso8601_date()
{
	time_t now_seconds = time(NULL);
	struct tm now;
	gmtime_r(&now_seconds, &now);
	char str[32];
	strftime(str, 32, "%Y%m%dT%H%M%SZ", &now);
	return str;
}


