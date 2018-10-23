#include "commonlib.h"

char* byteToString(char *pDest, char *pSrc, int srcLen) {
	memset(pDest, '\0', srcLen + 1);
	strncpy(pDest, pSrc, srcLen);
		
	return pDest;
}

float timeDeltaMillis(long startTime, long endTime) {
	return (endTime - startTime) / 1000.0f;
}

long long currentTimeMillis() {
	struct timeval stTimeval;
	long long msec = 0, usec = 0;
	
	gettimeofday(&stTimeval, NULL);
	
/*	msec = stTimeval.tv_sec * 1000;
	usec = stTimeval.tv_usec / 1000;
	
	msec = stTimeval.tv_sec * 1000L;
	usec = stTimeval.tv_usec / 1000L;
*/	
	msec = stTimeval.tv_sec * 1000LL;
	usec = stTimeval.tv_usec / 1000LL;

	return msec + usec;
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

void pushpopxy(int x, int y) {
	static COORD stSavedPos = { 1, 1 };
	COORD curPos;
	
	if (x == 0 || y == 0) /* pop and move */
	{
		gotoxy(stSavedPos.X, stSavedPos.Y);	
		stSavedPos.X = 0;
		stSavedPos.Y = 0;
	}
	else /* push and move */
	{
		stSavedPos = getxy();
		gotoxy(x, y);
	}
}

void gotoxy(int x, int y) {
    COORD stPos = { x - 1, y - 1 };
	
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), stPos);
}

COORD getxy() {
    COORD stPos;
    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    stPos.X = info.dwCursorPosition.X + 1;
    stPos.Y = info.dwCursorPosition.Y + 1;
	
    return stPos;
}