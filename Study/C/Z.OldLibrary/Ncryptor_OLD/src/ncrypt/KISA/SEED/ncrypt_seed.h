#ifndef __NCRYPT_SEED_H__
#define __NCRYPT_SEED_H__

#include "ncrypt_typedef.h"
#include "KISA/KISA_typedef.h"

static void Encrypt_SEED(BYTE *pbBlock, int szBlock, BYTE *pbRoundKey, int szRoundKey);
static void Decrypt_SEED(BYTE *pbBlock, int szBlock, BYTE *pbRoundKey, int szRoundKey);
static int CheckArgs(NcryptArgs *pstArgs);

int Ncrypt_SEED(NcryptArgs *pstArgs);

#endif