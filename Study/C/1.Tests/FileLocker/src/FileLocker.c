#include "FileLocker.h"

//
// [ FileLocker.c Info ]
// FileLocker Use AES-256 for en/decryption.
// Key length is 32byte(256bit) cipher block size is 16byte.
// File chunk-block size is 1024byte.
//
// (1) Encryption
// 1. Read File 1 ~ sizeof(FileBlock.abData) (as much as file remains, max:sizeof(FileBlock.abData))
// 2. Fill the struct FileBlock (Fill header, data and spare)
// 3. Aes Encryption
// 4. Write to output file
//
// (2) Decryption
// 1. Read File 1 ~ sizeof(FileBlock) (as much as file remains, max:sizeof(FileBlock))
// 2. Aes Decryption
// 3. Fill the struct FileBlock
// 4. Write to output file
//

// Print how to use program
void PrintUsage()
{
    fprintf(stdout, "FileLocker '-type' -key(1~32len)' 'File Name'\n");
    fprintf(stdout, "1. -type\n");
    fprintf(stdout, " 1) -l : lock\n");
    fprintf(stdout, " 2) -u : unlock\n");
    fprintf(stdout, "2. -key\n");
    fprintf(stdout, " 1) ex) '12345678' : 1~32length of file locking key\n");
}

// Adjusting key to 32byte fixed length
int KeyAdjust(BYTE abKeyOut[32], BYTE *pbKeyIn)
{
    size_t szKeyIn;

    if (pbKeyIn == NULL) szKeyIn = 0;

    szKeyIn = strlen(pbKeyIn);
    memset(abKeyOut, 0x00, 32);
    memcpy(abKeyOut, pbKeyIn, max(szKeyIn, 32));

    return 0;
}

int OpenAndCreateFile(FILE **pbOpenFile, FILE **pbCreateFile,
                      BYTE *pbOpenFileName, int isEncryption)
{
    static const BYTE *PB_LOCKFILE_EXT   = ".lock";
    static const BYTE *PB_UNLOCKFILE_EXT = ".unlock";
    BYTE abCreateFileName[128];

    if (isEncryption)
    {
        // Openfile.txt.lock
        strcpy(abCreateFileName, pbOpenFileName);
        strcat(abCreateFileName, PB_LOCKFILE_EXT);
    }
    else
    {
        // Openfile.txt.unlock
        strcpy(abCreateFileName, pbOpenFileName);
        strcat(abCreateFileName, PB_UNLOCKFILE_EXT);
    }

    if ((*pbOpenFile = fopen(pbOpenFileName, "rb")) == NULL)
    {
        fprintf(stdout, "[ERR] Fail to open file '%s'.\n", pbOpenFileName);
        return -1;
    }
    
    if ((*pbCreateFile = fopen(abCreateFileName, "wb")) == NULL)
    {
        fprintf(stdout, "[ERR] Fail to create file '%s'.\n", abCreateFileName);
        return -1;
    }

    return 0;
}

size_t Locker_GetFileSize(FILE *pFileIn)
{
    size_t szInFile = -1;
    size_t nCurPos = 0;

    if (pFileIn == NULL) return -1;

    fseek(pFileIn, 0, SEEK_CUR);
    nCurPos = ftell(pFileIn);
    fseek(pFileIn, 0, SEEK_END);
    szInFile = ftell(pFileIn);
    fseek(pFileIn, nCurPos, SEEK_SET);

    return szInFile;
}

int EncryptFileBlock(BYTE *pbOutStream, size_t szOutStream, FileBlock *pstInBlock, BYTE abAesKey[32])
{
    NCRYPT_ALGO_AES_ARGS stAes;
    BYTE                 arIV[16];

    memset(&stAes, 0x00, sizeof(stAes));
    memset(arIV, 0x00, sizeof(arIV));

    fprintf(stdout, "\nBeforeAES(len:%d)\n", sizeof(FileBlock));
    PrintStreamToHexa((BYTE*)pstInBlock, sizeof(FileBlock));
    fprintf(stdout, "\n");
    
    stAes.stArgs.isEncrypt  = 1;
    stAes.stArgs.eAlgo      = NCRYPT_ALGO_AES;
    stAes.stArgs.ePadd      = NCRYPT_PADD_NULL;
    stAes.stArgs.eMode      = NCRYPT_MODE_CBC;
    stAes.stArgs.pbIV       = arIV;
    stAes.stArgs.szIV       = sizeof(arIV);
    stAes.stArgs.pbKey      = abAesKey;
    stAes.stArgs.szKey      = 32;
    stAes.stArgs.pbInStream = (BYTE*)pstInBlock;
    stAes.stArgs.szInStream = sizeof(FileBlock);
    stAes.stArgs.pbOutBuf   = pbOutStream;
    stAes.stArgs.szOutBuf   = szOutStream;
    stAes.eKeyBit           = NCRYPT_AES_KEYBIT_256;

    //return Ncrypt(&stAes, sizeof(stAes));
    int rt = Ncrypt(&stAes, sizeof(stAes));

    fprintf(stdout, "\nAfterAES-Enc(len:%d)\n", rt);
    PrintStreamToHexa(pbOutStream, rt);
    fprintf(stdout, "\n");

    BYTE arTempBlock[szOutStream];

    memset(&stAes, 0x00, sizeof(stAes));
    memset(arIV, 0x00, sizeof(arIV));
    stAes.stArgs.isEncrypt  = 0;
    stAes.stArgs.eAlgo      = NCRYPT_ALGO_AES;
    stAes.stArgs.ePadd      = NCRYPT_PADD_NULL;
    stAes.stArgs.eMode      = NCRYPT_MODE_CBC;
    stAes.stArgs.pbIV       = arIV;
    stAes.stArgs.szIV       = sizeof(arIV);
    stAes.stArgs.pbKey      = abAesKey;
    stAes.stArgs.szKey      = 32;
    stAes.stArgs.pbInStream = pbOutStream;
    stAes.stArgs.szInStream = rt;
    stAes.stArgs.pbOutBuf   = arTempBlock;
    stAes.stArgs.szOutBuf   = sizeof(arTempBlock);
    stAes.eKeyBit           = NCRYPT_AES_KEYBIT_256;

    rt = Ncrypt(&stAes, sizeof(stAes));

    fprintf(stdout, "\nAfterAES-Dec(len:%d)\n", rt);
    PrintStreamToHexa(arTempBlock, rt);
    fprintf(stdout, "\n");

    return rt;
}

int DecryptStream(FileBlock *pstOutBlock, BYTE *pbInStream, size_t szInStream, BYTE abAesKey[32])
{
    NCRYPT_ALGO_AES_ARGS stAes;
    BYTE                 arIV[16];
    BYTE                 arTempBlock[szInStream];

    memset(&stAes, 0x00, sizeof(stAes));
    memset(arIV, 0x00, sizeof(arIV));

    fprintf(stdout, "\nBeforeAES(len:%d)\n", szInStream);
    PrintStreamToHexa((BYTE*)pbInStream, szInStream);
    fprintf(stdout, "\n");
    
    stAes.stArgs.isEncrypt  = 0;
    stAes.stArgs.eAlgo      = NCRYPT_ALGO_AES;
    stAes.stArgs.ePadd      = NCRYPT_PADD_NULL;
    stAes.stArgs.eMode      = NCRYPT_MODE_CBC;
    stAes.stArgs.pbIV       = arIV;
    stAes.stArgs.szIV       = sizeof(arIV);
    stAes.stArgs.pbKey      = abAesKey;
    stAes.stArgs.szKey      = 32;
    stAes.stArgs.pbInStream = pbInStream;
    stAes.stArgs.szInStream = szInStream;
    stAes.stArgs.pbOutBuf   = arTempBlock;
    stAes.stArgs.szOutBuf   = sizeof(arTempBlock);
    stAes.eKeyBit           = NCRYPT_AES_KEYBIT_256;

    int rt = Ncrypt(&stAes, sizeof(stAes));

    fprintf(stdout, "\nAfterAES(len:%d)\n", rt);
    PrintStreamToHexa(arTempBlock, rt);
    fprintf(stdout, "\n");

    memcpy((BYTE*)pstOutBlock, arTempBlock, sizeof(FileBlock));

    return sizeof(FileBlock);
}

size_t MakeFileBlockFromStream(FileBlock *pstOutBlock, BYTE *pFromStream, size_t szFromStream)
{
    size_t szData = min(sizeof(pstOutBlock->abData), szFromStream);
    
    pstOutBlock->abHeader[0] = (BYTE)0x01;              // Header : SOH
    sprintf(&pstOutBlock->abHeader[1], "%04d", szData); // Header : szData
    pstOutBlock->abHeader[5] = (BYTE)0x02;              // Header : STX
    memcpy(pstOutBlock->abData, pFromStream, szData);   // Data
    pstOutBlock->abSpare[0] = (BYTE)0xff;               // Spare
    pstOutBlock->abSpare[1] = (BYTE)0x03;               // Spare : ETX

    return szData;
}

size_t MakeSteramFromFileBlock(BYTE *pbOutStream, FileBlock *pstFromBlock)
{
    size_t szData = -1;
    size_t szNewHeader = sizeof(pstFromBlock->abHeader) + 1; // atoi()를 위해 '\0'추가
    BYTE abHeader[szNewHeader];

    memset(abHeader, 0x00, szNewHeader);
    memcpy(abHeader, pstFromBlock->abHeader, sizeof(pstFromBlock->abHeader));
    szData = atoi(abHeader);                                                    // Header : szData
    memcpy(pbOutStream, pstFromBlock->abData, szData);                          // Data

    return szData;
}

// Locker(lock/unlock) Logic
int FileLocker(BYTE *pbWorkType, BYTE *pbInKey, BYTE *pbInFileName)
{
    BYTE    abAesKey[32];
    int     isEncryption = 0;
    int     rtVal = -1;

    if (pbWorkType == NULL) return -1;

    if (strcmp(pbWorkType, "-l") == 0) // lock
    {
        isEncryption = 1;
    }
    else if (strcmp(pbWorkType, "-u") == 0) // unlock
    {
        isEncryption = 0;
    }
    else // error
    {
        fprintf(stdout, "[ERR] Unknown worktype '%s'.\n", pbWorkType);
        return -1;
    }

    if ((rtVal = KeyAdjust(abAesKey, pbInKey)) == -1)
    {
        fprintf(stdout, "[ERR] KeyAdjust()\n");
        return rtVal;
    }

    rtVal = (isEncryption ? FileLock(abAesKey, pbInFileName, abAesKey) : FileUnlock(abAesKey, pbInFileName, abAesKey));

    return rtVal;
}

// Lock
int FileLock(BYTE *pbInKey, BYTE *pbInFileName, BYTE abAesKey[32])
{
    FILE    *pFileIn = NULL, *pFileOut = NULL;
    size_t  szInFile, szRemain;
    int     nBlockCnt, rtVal;

    if ((rtVal = OpenAndCreateFile(&pFileIn, &pFileOut, pbInFileName, 1)) == -1)
    {
        fprintf(stdout, "[ERR] OpenAndCreateFile()\n", pbInFileName);
        goto END;
    }

    szInFile = Locker_GetFileSize(pFileIn);
    szRemain = szInFile;
    nBlockCnt = 0;

    {
        int         szStream = sizeof(FileBlock) + (sizeof(FileBlock) % 16); // AES block is 16byte
        BYTE        abStream[szStream];
        FileBlock   stBlock;
        size_t      szRead;

        while (szRemain > 0)
        {
            memset(abStream, 0x00, sizeof(abStream));
            memset(&stBlock, 0x00, sizeof(stBlock));
            
            if ((szRead = fread(abStream, sizeof(BYTE), min(szRemain, sizeof(stBlock.abData)), pFileIn)) < 1) break;
            
            if ((rtVal = MakeFileBlockFromStream(&stBlock, abStream, szRead)) == -1)
            {
                fprintf(stdout, "[ERR] MakeFileBlockFromStream()\n");
                goto END;
            }

            //fprintf(stdout, "\n< szRemain:%ld / read:%ld / rtVal:%d >", szRemain, szRead, rtVal);
            //fprintf(stdout, "\n[BGN ST BLK](%d)\n", nBlockCnt);
            //PrintStreamToHexa((BYTE*)&stBlock, sizeof(stBlock));
            //fprintf(stdout, "\n[END ST BLK]\n");

            if ((rtVal = EncryptFileBlock(abStream, szStream, &stBlock, abAesKey)) == -1)
            {
                fprintf(stdout, "[ERR] EncryptFileBlock()\n");
                goto END;
            }

            //fprintf(stdout, "\n< szRemain:%ld / read:%ld / rtVal:%d >", szRemain, szRead, rtVal);
            //fprintf(stdout, "\n[BGN AES BLK](%d)\n", nBlockCnt);
            //PrintStreamToHexa(abStream, rtVal);
            //fprintf(stdout, "\n[END AES BLK]\n");
            
            if ((rtVal = fwrite(abStream, sizeof(BYTE), rtVal, pFileOut)) == -1)
            {
                fprintf(stdout, "[ERR] Fail to write file.\n");
                goto END;
            }

            szRemain -= szRead;
            ++nBlockCnt;
        }
    }

END:
    if (pFileIn != NULL)  fclose(pFileIn);
    if (pFileOut != NULL) fclose(pFileOut);
    return rtVal;
}

// Unlock
int FileUnlock(BYTE *pbInKey, BYTE *pbInFileName, BYTE abAesKey[32])
{
    FILE    *pFileIn = NULL, *pFileOut = NULL;
    size_t  szInFile, szRemain;
    int     nBlockCnt, rtVal;

    if ((rtVal = OpenAndCreateFile(&pFileIn, &pFileOut, pbInFileName, 0)) == -1)
    {
        fprintf(stdout, "[ERR] OpenAndCreateFile()\n", pbInFileName);
        goto END;
    }

    szInFile = Locker_GetFileSize(pFileIn);
    szRemain = szInFile;
    nBlockCnt = 0;

    {
        int         szStream = sizeof(FileBlock) + (sizeof(FileBlock) % 16); // AES block is 16byte
        BYTE        abStream[szStream];
        FileBlock   stBlock;
        size_t      szRead;

        while (szRemain > 0)
        {
            memset(abStream, 0x00, sizeof(abStream));
            memset(&stBlock, 0x00, sizeof(stBlock));
            
            if ((szRead = fread(abStream, sizeof(BYTE), szStream, pFileIn)) != szStream) break;

            if ((rtVal = DecryptStream(&stBlock, abStream, szStream, abAesKey)) == -1)
            {
                fprintf(stdout, "[ERR] DecryptStream()\n");
                goto END;
            }

            //fprintf(stdout, "\n< szRemain:%ld / read:%ld / rtVal:%d >", szRemain, szRead, rtVal);
            //fprintf(stdout, "\n[BGN ST BLK](%d)\n", nBlockCnt);
            //PrintStreamToHexa((BYTE*)&stBlock, rtVal);
            //fprintf(stdout, "\n[END ST BLK]\n");

            if ((rtVal = MakeSteramFromFileBlock(abStream, &stBlock)) == -1)
            {
                fprintf(stdout, "[ERR] MakeSteramFromFileBlock()\n");
                goto END;
            }

            //fprintf(stdout, "\n< szRemain:%ld / read:%ld / rtVal:%d >", szRemain, szRead, rtVal);
            //fprintf(stdout, "\n[BGN Write BLK](%d)\n", nBlockCnt);
            //PrintStreamToHexa(abStream, rtVal);
            //fprintf(stdout, "\n[END Write AES BLK]\n");
            
            if ((rtVal = fwrite(abStream, sizeof(BYTE), rtVal, pFileOut)) == -1)
            {
                fprintf(stdout, "[ERR] Fail to write file.\n");
                goto END;
            }

            szRemain -= szRead;
            ++nBlockCnt;
        }
    }

END:
    if (pFileIn != NULL)  fclose(pFileIn);
    if (pFileOut != NULL) fclose(pFileOut);
    return rtVal;
}

// Main
int main(int argc, char **argv)
{
    int rtVal = 0;

    if (argc != 4)
    {
        PrintUsage();
        return -1;
    }

    fprintf(stdout, "Type : %s\n", argv[1]);
    fprintf(stdout, "Key  : %s\n", argv[2]);
    fprintf(stdout, "File : %s\n", argv[3]);

    if ((rtVal = FileLocker(argv[1], argv[2], argv[3])) == -1)
    {
        fprintf(stdout, "[ERR] FileLocker failed.\n");
        return rtVal;
    }
    else
    {
        fprintf(stdout, "[OK] FileLocker Complete.\n");
        return rtVal;
    }

    return 0;
}