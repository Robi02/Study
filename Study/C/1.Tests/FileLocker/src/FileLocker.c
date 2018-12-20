#include "FileLocker.h"

void PrintUsage()
{
    fprintf(stdout, "FileLocker '-type' -key(1~24len)' 'File Name'\n");
    fprintf(stdout, "1. -type\n");
    fprintf(stdout, " 1) -l : lock\n");
    fprintf(stdout, " 2) -u : unlock\n");
    fprintf(stdout, "2. -key\n");
    fprintf(stdout, " 1) ex) '12345678' : 1~24length of file locking key\n");
}

int FileLock(char *pWorkType, char *pKey, char *pFileName)
{
    int                     isLock = 0;
    NCRYPT_ALGO_AES_ARGS    stAes;
    char                    fileTag[16];
    char                    arKey[24];
    char                    arIV[24];
    char                    arFileBuf[2048];
    char                    arOuFileBuf[2048];
    char                    arOuFileName[strlen(pFileName) + sizeof(fileTag)];
    FILE                    *pFile = NULL, *pOutFile = NULL;
    int                     szFile = 0;
    int                     idx = 0;

    isLock = (strcmp(pWorkType, "-l") == 0 ? 1 : 0);
    (isLock == 1 ? strcpy(fileTag, "_lock") : strcpy(fileTag, "_unlock"));

    if ((pFile = fopen(pFileName, "rb")) == NULL)
    {
        fprintf(stdout, "pFile is NULL.\n");
        return -1;
    }

    strcpy(arOuFileName, strcat(pFileName, fileTag));

    if ((pOutFile = fopen(arOuFileName, "wb")) == NULL)
    {
        fprintf(stdout, "Cannot create new file.\n");
        return -1;
    }

    memset(arKey, 0x13, sizeof(arKey));
    memcpy(arKey, pKey, strlen(pKey));

    fseek(pFile, 0, SEEK_END);
    szFile = ftell(pFile);
    rewind(pFile);

    while (idx < szFile)
    {
        int szRemainFile = ftell(pFile);
        int blockLen = min(sizeof(arFileBuf), szRemainFile);
        int cryptLen = 0;

        memset(arFileBuf, 0x00, sizeof(arFileBuf));
        memset(arOuFileBuf, 0x00, sizeof(arOuFileBuf));
        fread(arFileBuf, sizeof(char), blockLen, pFile);
        
        memset(arIV, 0x00, sizeof(arIV));
        stAes.stArgs.isEncrypt  = isLock;
        stAes.stArgs.eAlgo      = NCRYPT_ALGO_AES;
        stAes.stArgs.ePadd      = NCRYPT_PADD_NULL;
        stAes.stArgs.eMode      = NCRYPT_MODE_CBC;
        stAes.stArgs.pbKey      = arKey;
        stAes.stArgs.szKey      = sizeof(arKey);
        stAes.stArgs.pbIV       = arIV;
        stAes.stArgs.szIV       = sizeof(arIV);
        stAes.stArgs.pbInStream = arFileBuf;
        stAes.stArgs.szInStream = sizeof(arFileBuf);
        stAes.stArgs.pbOutBuf   = arOuFileBuf;
        stAes.stArgs.szOutBuf   = sizeof(arOuFileBuf);
        stAes.eKeyBit           = NCRYPT_AES_KEYBIT_196; // (24bit)

        cryptLen = Ncrypt(&stAes, sizeof(stAes));
        fwrite(arOuFileBuf, sizeof(char), cryptLen, pOutFile);
        idx += cryptLen;
    }

    fprintf(stdout, "Out File Size : %d\n", idx);

    fclose(pFile);
    fclose(pOutFile);

    return idx;
}

int main(int argc, char **argv)
{
    int result = 0;
    
    if (argc != 4)
    {
        fprintf(stdout, "argc != 4 (argc:%d)\n", argc);
        PrintUsage();
        return -1;
    }

    fprintf(stdout, "pWorkType : %s\n", argv[1]);
    fprintf(stdout, "pKey :      %s\n", argv[2]);
    fprintf(stdout, "pFileName : %s\n", argv[3]);

    if ((result = FileLock(argv[1], argv[2], argv[3])) < 1)
    {
        PrintUsage();
        return -1;
    }

    return result;
}