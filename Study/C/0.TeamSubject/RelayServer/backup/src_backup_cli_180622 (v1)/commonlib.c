#include "commonlib.h"

char* byteToString(char *pDest, char *pSrc, int srcLen) {
	memset(pDest, '\0', srcLen + 1);
	strncpy(pDest, pSrc, srcLen);
		
	return pDest;
}

float timeDeltaMillis(long startTime, long endTime) {
	return (endTime - startTime) / 1000.0f;
}

long currentTimeMillis() {
	struct timeval stTimeval;
	
	gettimeofday(&stTimeval, NULL);

	return (stTimeval.tv_sec * 1000) + (stTimeval.tv_usec / 1000);
}

char* getTime(char *pOutBuf, int option) {
	time_t timer;
	struct tm *pTime = NULL;
	int year = -1, month = -1, day = -1;
	int hour = -1, min = -1, sec = -1;
	
	timer = time(NULL);
	pTime = localtime(&timer);
	
	year = pTime->tm_year + 1900;
	month = pTime->tm_mon + 1;
	day = pTime->tm_mday;
	hour = pTime->tm_hour;
	min = pTime->tm_min;
	sec = pTime->tm_sec;
	
	switch (option)
	{
		case 0: /* YYMMDD */
			sprintf(pOutBuf, "%02d%02d%02d", year - 2000, month, day);
			break;
		case 1: /* YYYYMMDD */
			sprintf(pOutBuf, "%04d%02d%02d", year, month, day);
			break;
		case 2: /* hhmmss */
			sprintf(pOutBuf, "%02d%02d%02d", hour, min, sec);
			break;
		case 3: /* YYYYMMDDhhmmss */
			sprintf(pOutBuf, "%04d%02d%02d%02d%02d%02d", year, month, day, hour, min, sec);
			break;
		default: break;
	}
	
	return pOutBuf;
}