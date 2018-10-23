#ifndef __NCRYPTOR_MAIN_H__
#define __NCRYPTOR_MAIN_H__

#include "ncrypt_typedef.h"

static void GetNameFromIdx(char *pBuf, int szBuf, int idx);
static int NcryptorTest(NcryptArgs *pkArgs);
static int CheckArgs(int argc, char **argv, NcryptArgs *pkArgs);
static int CheckWorktype(NcryptArgs *pstArgs, char *pWorktype);
static void PrintUsage();

int main(int argc, char **argv);

#endif