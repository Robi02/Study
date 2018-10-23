#ifndef __NCRYPTOR_H__
#define __NCRYPTOR_H__

#include "ncrypt_typedef.h"

int Ncrypt(NCRYPT_WORKTYPE eWorktype, NCRYPT_ALGORITHM eAlgorithm, NCRYPT_MODE eMode, NCRYPT_PADDING ePadding,
           unsigned char *pInStream, int szInStream, unsigned char *pOutBuf, int szOutBuf,
           unsigned char *pKey, unsigned char *pIV);

#endif