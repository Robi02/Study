#ifndef __NCRYPT_BLOWFISH_H__
#define __NCRYPT_BLOWFISH_H__

#include "common_typedef.h"
#include "common_ncrypt.h"

typedef struct _NCRYPT_ALGO_BLOWFISH_ARGS
{
    NCRYPT_COMMON_BLOCK_ARGS stArgs;    // Base data for block ncryption.

} NCRYPT_ALGO_BLOWFISH_ARGS;

int Ncrypt_BLOWFISH(NCRYPT_ALGO_BLOWFISH_ARGS *pstArgs);

#endif