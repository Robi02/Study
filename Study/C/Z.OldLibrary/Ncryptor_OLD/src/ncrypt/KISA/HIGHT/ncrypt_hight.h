#ifndef __NCRYPT_HIGHT_H__
#define __NCRYPT_HIGHT_H__

#include "ncrypt_typedef.h"
#include "KISA/KISA_typedef.h"

static void Encrypt_HIGHT(BYTE *pbBlock, int szBlock, BYTE *pbRoundKey, int szRoundKey);
static void Decrypt_HIGHT(BYTE *pbBlock, int szBlock, BYTE *pbRoundKey, int szRoundKey);
static int CheckArgs(NcryptArgs *pstArgs);

int Ncrypt_HIGHT(NcryptArgs *pstArgs);

#endif