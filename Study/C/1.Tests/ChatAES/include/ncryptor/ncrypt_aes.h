#ifndef __NCRYPT_AES_H__
#define __NCRYPT_AES_H__

#include "common_typedef.h"
#include "common_ncrypt.h"

typedef enum _NCRYPT_AES_KEYBIT
{
    NCRYPT_AES_KEYBIT_128 = 0,  // 16byte key - 10round - 176byte roundkey
    NCRYPT_AES_KEYBIT_196 = 1,  // 24byte key - 12round - 208byte roundkey
    NCRYPT_AES_KEYBIT_256 = 2,  // 32byte key - 14round - 240byte roundkey
    NCRYPT_AES_KEYBIT_MAX

} NCRYPT_AES_KEYBIT;

typedef struct _NCRYPT_ALGO_AES_ARGS
{
    NCRYPT_COMMON_BLOCK_ARGS    stArgs;
    NCRYPT_AES_KEYBIT           eKeyBit;

} NCRYPT_ALGO_AES_ARGS;

// [Note] Init function for Roundkey.
// At the first call of Ncrypt_AES(), this Init_Function will be called automatically.
// So, you don't have to call Ncrypt_AES_Init() unless you whan to change for new key.
int Ncrypt_AES_Init(BYTE *pbKey, int szKey);
int Ncrypt_AES(NCRYPT_ALGO_AES_ARGS *pstArgs);

#endif