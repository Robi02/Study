#ifndef __NCRYPT_ARIA_H__
#define __NCRYPT_ARIA_H__

#include "ncrypt_typedef.h"
#include "KISA/KISA_typedef.h"

static void Encrypt_ARIA(BYTE *pbBlock, int szBlock, BYTE *pbRoundKey, int szRoundKey);
static void Decrypt_ARIA(BYTE *pbBlock, int szBlock, BYTE *pbRoundKey, int szRoundKey);
static int CheckArgs(NcryptArgs *pstArgs);

int Ncrypt_ARIA(NcryptArgs *pstArgs);

#endif