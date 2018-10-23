#ifndef __COMMONLIB_H__
#define __COMMONLIB_H__

#include "stdheader.h"

char* byteToString(char *pDest, char *pSrc, int srcLen);
float timeDeltaMillis(long startTime, long endTime);
long long currentTimeMillis();
char* getTime(char *pOutBuf, int option);
void pushpopxy(int x, int y);
void gotoxy(int x, int y);
COORD getxy();

#endif