#ifndef __BYTESTREAM_H__
#define __BYTESTREAM_H__

#include "common_define.h"

typedef struct _iobuf FILE;

int CountStreamInStream(const BYTE *pbIn, int szIn, const BYTE *pbFind, int szFind);
void PrintStreamToBinary(FILE *pFd, const BYTE *pbIn, int szIn);
void PrintStreamToHexa(const BYTE *pbStream, int szStream);
void PrintStreamToString(const BYTE *pbStream, int szStream);
void StreamFromString(BYTE *pbStream, int szStream, BYTE *pbStr);

#endif