#ifndef __NCRYPT_COMMON_H__
#define __NCRYPT_COMMON_H__

#include "KISA/KISA_typedef.h"
#include "ncrypt_typedef.h"

static int CheckCommonArgs(NcryptArgs *pstArgs, int szBlock);
static int GetBlockFromStream(BYTE *pbOutBlock, int szOutBlock, BYTE *pbInStream, int szRemain, NCRYPT_PADDING ePadding);
static int XorStream(BYTE *pbStream_L, int szStream_L, const BYTE *pbStream_R, const int szStream_R);

int UpdateKeys(NcryptArgs *pstArgs, BYTE *pbKey, int szKey, BYTE *pbRoundKey, int szRoundKey,
               void (*pKeyFunc)(BYTE*, int, BYTE*, int), int *pIsKeyInserted);
int EnDecryption(NcryptArgs *pstArgs, int szBlock, BYTE *pbRoundKey, int szRoundKey,
                 void (*pEncFunc)(BYTE*, int, BYTE*, int), void (*pDecFunc)(BYTE*, int, BYTE*, int));
void PrintStreamToHexa(const unsigned char *pStream, int szStream);
void PrintStreamToString(const unsigned char *pStream, int szStream);
void GenerateStream(unsigned char *pStream, int szStream, int optRandom);
void GenerateStreamFromString(unsigned char *pStream, int szStream, unsigned char *pStr);

#endif