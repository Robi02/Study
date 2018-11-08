#ifndef __NCRYPT_ARIA_H__
#define __NCRYPT_ARIA_H__

#include "common_ncrypt.h"

typedef enum _NCRYPT_ARIA_ROUNDS
{
    NCRYPT_ARIA_ROUND12 = 0,    // 12round - 16byte key - 208byte roundkey
    NCRYPT_ARIA_ROUND14 = 1,    // 14round - 24byte key - 360byte roundkey
    NCRYPT_ARIA_ROUND16 = 2,    // 16round - 32byte key - 544byte roundkey
    NCRYPT_ARIA_ROUND_MAX

} NCRYPT_ARIA_ROUNDS;

typedef struct _NCRYPT_ALGO_ARIA_ARGS
{
    NCRYPT_COMMON_BLOCK_ARGS stArgs;    // Base data for block ncryption.
    NCRYPT_ARIA_ROUNDS eRounds;         // ARIA round option.

} NCRYPT_ALGO_ARIA_ARGS;

// [Note] Init function for Roundkey.
// At the first call of Ncrypt_ARIA(), this Init_Function will be called automatically.
// So, you don't have to call Ncrypt_ARIA_Init() unless you whan to change for new key.
int Ncrypt_ARIA_Init(NCRYPT_ARIA_ROUNDS eRounds, BYTE *pbKey, int szKey);
int Ncrypt_ARIA(NCRYPT_ALGO_ARIA_ARGS *pstArgs);

#endif