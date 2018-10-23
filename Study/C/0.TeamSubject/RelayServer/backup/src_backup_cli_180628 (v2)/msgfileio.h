#ifndef __MSGFILEIO_H__
#define __MSGFILEIO_H__

#include "stdheader.h"

FILE* openMsgFile(char *pFileName, char *pOption);
int readMsgLine(FILE *pFile, char *pOutLineBuf, int outBufSize);
int writeMsg(FILE *pFile, char *pData, int lineAtFile, int useNewLine);
int closeMsgFile(FILE *pFile);
int writeLog(FILE *pFile, char *pData, int szData, char *pPrefixStr, char *pSuffixStr);

#endif