#ifndef __NCRYPT_HIGHT_H__
#define __NCRYPT_HIGHT_H__

#include "common_ncrypt.h"

typedef struct _NCRYPT_ALGO_HIGHT_ARGS
{
    NCRYPT_COMMON_BLOCK_ARGS stArgs;

} NCRYPT_ALGO_HIGHT_ARGS;

// [Note] Init function for Roundkey.
// At the first call of Ncrypt_HIGHT(), this Init_Function will be called automatically.
// So, you don't have to call Ncrypt_HIGHT_Init() unless you whan to change for new key.
int Ncrypt_HIGHT_Init(BYTE *pbKey, int szKey);
int Ncrypt_HIGHT(NCRYPT_ALGO_HIGHT_ARGS *pstArgs);

#endif