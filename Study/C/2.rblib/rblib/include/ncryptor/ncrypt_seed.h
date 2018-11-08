#ifndef __NCRYPT_SEED_H__
#define __NCRYPT_SEED_H__

#include "common_ncrypt.h"

typedef struct _NCRYPT_ALGO_SEED_ARGS
{
    NCRYPT_COMMON_BLOCK_ARGS stArgs;

} NCRYPT_ALGO_SEED_ARGS;

// [Note] Init function for Roundkey.
// At the first call of Ncrypt_SEED(), this Init_Function will be called automatically.
// So, you don't have to call Ncrypt_SEED_Init() unless you whan to change for new key.
int Ncrypt_SEED_Init(BYTE *pbKey, int szKey);
int Ncrypt_SEED(NCRYPT_ALGO_SEED_ARGS *pstArgs);

#endif