#ifndef __FILE_LOCKER_H__
#define __FILE_LOCKER_H__

#include "lib_ncryptor.h"
#include "lib_ncoder.h"
#include "bytestream.h"

typedef struct stFileBlock
{
    BYTE abHeader[6];
    BYTE abData[1024];
    BYTE abSpare[2];

} FileBlock;

void PrintUsage();
int KeyAdjust(BYTE abKeyOut[32], BYTE *pbKeyIn);
int OpenAndCreateFile(FILE **pbOpenFile, FILE **pbCreateFile,
                      BYTE *pbOpenFileName, int isEncryption);
size_t Locker_GetFileSize(FILE *pFileIn);
int EncryptFileBlock(BYTE *pbOutStream, size_t szOutStream, FileBlock *pstInBlock, BYTE abAesKey[32]);
int DecryptStream(FileBlock *pstOutBlock, BYTE *pbInStream, size_t szInStream, BYTE abAesKey[32]);
size_t MakeFileBlockFromStream(FileBlock *pstOutBlock, BYTE *pFromStream, size_t szFromStream);
size_t MakeSteramFromFileBlock(BYTE *pbOutStream, FileBlock *pstFromBlock);
int FileLocker(BYTE *pbWorkType, BYTE *pbInKey, BYTE *pbInFileName);
int FileLock(BYTE *pbInKey, BYTE *pbInFileName, BYTE abAesKey[32]);
int FileUnlock(BYTE *pbInKey, BYTE *pbInFileName, BYTE abAesKey[32]);

#endif