#ifndef __COMMONLIB_H__
#define __COMMONLIB_H__

#include "stdheader.h"

char* byteToString(char *pDest, char *pSrc, int srcLen);
float timeDeltaMillis(long startTime, long endTime);
long currentTimeMillis();
char* getTime(char *pOutBuf, int option);

#endif