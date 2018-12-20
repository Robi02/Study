#ifndef __NCRYPT_DES_H__
#define __NCRYPT_DES_H__

#include "common_ncrypt.h"

typedef struct _NCRYPT_ALGO_DES_ARGS
{
    NCRYPT_COMMON_BLOCK_ARGS stArgs;    // Base data for block ncryption.
    int isTripleDES;                    // '1' is triple DES, use 24byte key. others are 8byte key.

} NCRYPT_ALGO_DES_ARGS;

// [Note] Ncrypt_DES does not need 'Init_Function'.
// That's because RoundKey will be made from 'function_des()' function.
int Ncrypt_DES(NCRYPT_ALGO_DES_ARGS *pstArgs);

#endif