#include "ClockService.h"

#include <Arduino.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

void ClockService::initializeIfNeeded()
{
	time_t now;
	time(&now);

	struct tm current;
	localtime_r(&now, &current);

	if (current.tm_year + 1900 >= 2025)
		return;

	char monthStr[4] = {};
	int day;
	int year;
	int hour;
	int minute;
	int second;

	sscanf(__DATE__, "%3s %d %d",
		   monthStr, &day, &year);

	sscanf(__TIME__, "%d:%d:%d",
		   &hour, &minute, &second);

	static const char *months[] = {
		"Jan", "Feb", "Mar", "Apr",
		"May", "Jun", "Jul", "Aug",
		"Sep", "Oct", "Nov", "Dec"};

	int month = 0;

	for (int i = 0; i < 12; i++)
	{
		if (strcmp(monthStr, months[i]) == 0)
		{
			month = i;
			break;
		}
	}

	struct tm compileTime = {};

	compileTime.tm_year = year - 1900;
	compileTime.tm_mon = month;
	compileTime.tm_mday = day;
	compileTime.tm_hour = hour;
	compileTime.tm_min = minute;
	compileTime.tm_sec = second;
	compileTime.tm_isdst = -1;

	const time_t compiled = mktime(&compileTime);

	struct timeval tv = {};
	tv.tv_sec = compiled;

	settimeofday(&tv, nullptr);
}