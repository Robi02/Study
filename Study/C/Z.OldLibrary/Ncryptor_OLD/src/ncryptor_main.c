#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rbmath.h"
#include "ncrypt.h"
#include "ncrypt_common.h"
#include "ncryptor_define.h"
#include "ncryptor_main.h"

static void GetNameFromIdx(char *pBuf, int szBuf, int idx)
{
    memset(pBuf, 0x00, szBuf);

    switch ( idx )
    {
        default: GenerateStreamFromString(pBuf, szBuf, "ERROR");
        break;
        /* Algorithm */
        case 0: GenerateStreamFromString(pBuf, szBuf, "SEED");
        break;
        case 1: GenerateStreamFromString(pBuf, szBuf, "HIGHT");
        break;
        case 2: GenerateStreamFromString(pBuf, szBuf, "ARIA");
        break;
        /* Mode */
        case 10: GenerateStreamFromString(pBuf, szBuf, "ECB");
        break;
        case 11: GenerateStreamFromString(pBuf, szBuf, "CBC");
        break;
        /* Padding */
        case 100: GenerateStreamFromString(pBuf, szBuf, "NULL");
        break;
        case 101: GenerateStreamFromString(pBuf, szBuf, "PKCS");
        break;
    }
}

static int NcryptorTest(NcryptArgs *pstArgs)
{
    NCRYPT_ALGORITHM       eAlgo = pstArgs->eAlgorithm;  /* Cipher Algorithm */
    const NCRYPT_MODE      eMode = pstArgs->eMode;       /* Mode option */
    const NCRYPT_PADDING   ePadd = pstArgs->ePadding;    /* Padding option */
    const unsigned char    *pStr = pstArgs->pInStream;   /* Plane Stream */
    
    int szInBuf = strlen(pStr);
    unsigned char inBuf[szInBuf];
    
    int szEnBuf = between(32, szInBuf * 2, szInBuf * 2);
    unsigned char enBuf[szEnBuf];
    
    int szDeBuf = szEnBuf;
    unsigned char deBuf[szDeBuf];

    unsigned char *pKey = pstArgs->pKey; /* User key */
    int szKey = strlen(pKey);
    
    unsigned char pIV[16]  = { 0x00, }; /* Initial Vector */
    int szIV = sizeof(pIV);
    
    int rtVal = 0;
    char pAlgoName[64], pModeName[64], pPaddName[64];

    memcpy(inBuf, pStr, szInBuf);
    memset(enBuf, 0x00, szEnBuf);
    memset(deBuf, 0x00, szDeBuf);

    GetNameFromIdx(pAlgoName, sizeof(pAlgoName), eAlgo);
    GetNameFromIdx(pModeName, sizeof(pModeName), eMode + 10);
    GetNameFromIdx(pPaddName, sizeof(pPaddName), ePadd + 100);
    
    fprintf(stdout, "\n================================================================\n");
    fprintf(stdout, "\n [ (%s / %s / %s) Initializing... ] \n", pAlgoName, pModeName, pPaddName);

    fprintf(stdout, "\n- Key (%dByte - String) : \n", szKey);
    PrintStreamToString(pKey, szKey);
    fprintf(stdout, "\n- Key (%dByte - Hexa) : \n", szKey);
    PrintStreamToHexa(pKey, szKey);

    if ( eMode == NM_CBC )
    {
        GenerateStream(pIV, szIV, 0x00);
        fprintf(stdout, "\n- IV (%dByte - String) : \n", szIV);
        PrintStreamToString(pIV, szIV);
        fprintf(stdout, "\n- IV (%dByte - Hexa) : \n", szIV);
        PrintStreamToHexa(pIV, szIV);
    }

    fprintf(stdout, "\n- Plane Stream (%dByte - String) : \n", szInBuf);
    PrintStreamToString(inBuf, szInBuf);
    fprintf(stdout, "\n- Plane Stream (%dByte - Hexa) : \n", szInBuf);
    PrintStreamToHexa(inBuf, szInBuf);

    fprintf(stdout, "\n================================================================\n");
    fprintf(stdout, "\n [ Encrypting... ] \n", szKey);

    if ( (rtVal = Ncrypt(NW_ENCRYPT, eAlgo, eMode, ePadd, inBuf, szInBuf, enBuf, szEnBuf, pKey, pIV)) < 0 )
    {
        LOG("Ncrypt(ENCRYPT) error while ENCRYPT... (return:%d)\n", rtVal);
        return -1;
    }
    
    fprintf(stdout, "\n- Cipher Stream (%dByte - String) : \n", rtVal);
    PrintStreamToString(enBuf, rtVal);
    fprintf(stdout, "\n- Cipher Stream (%dByte - Hexa) : \n", rtVal);
    PrintStreamToHexa(enBuf, rtVal);

    fprintf(stdout, "\n================================================================\n");
    fprintf(stdout, "\n [ Decrypting... ] \n", szKey);

    if ( (rtVal = Ncrypt(NW_DECRYPT, eAlgo, eMode, ePadd, enBuf, rtVal, deBuf, szDeBuf, pKey, pIV)) < 0 )
    {
        LOG("Ncrypt() error while DECRYT... (return:%d)\n", rtVal);
        return -1;
    }
    
    fprintf(stdout, "\n- Plane Stream (%dByte - String) : \n", rtVal);
    PrintStreamToString(deBuf, rtVal);
    fprintf(stdout, "\n- Plane Stream (%dByte - Hexa) : \n", rtVal);
    PrintStreamToHexa(deBuf, rtVal);
    fprintf(stdout, "\n================================================================\n");

    return rtVal;
}

static int CheckArgs(int argc, char **argv, NcryptArgs *pstArgs)
{
    int rtVal;

    if ( pstArgs == NULL )
    {
        LOG("pstArgs is NULL. (pstArgs:0x%p)\n", pstArgs);
        return -1;
    }

    if ( argc != 7 )
    {
        PrintUsage();
        return -1;
    }

    if ( argv[1] != NULL )
    {
        if ( strcmp(argv[1], "-e") == 0 )
        {
            pstArgs->eWorktype = NW_ENCRYPT;
        }
        else if ( strcmp(argv[1], "-d") == 0 )
        {
            pstArgs->eWorktype = NW_DECRYPT;
        }
        else
        {
            fprintf(stderr, "Unkown worktype '%s'\n", argv[1]);
            return -1;
        }
    }
    else
    {
        PrintUsage();
        return -1;
    }

    if ( argv[2] != NULL )
    {
        if ( strcmp(argv[2], "-seed") == 0 )
        {
            pstArgs->eAlgorithm = NA_SEED;
        }
        else if ( strcmp(argv[2], "-hight") == 0 )
        {
            pstArgs->eAlgorithm = NA_HIGHT;
        }
        else if ( strcmp(argv[2], "-aria") == 0 )
        {
            pstArgs->eAlgorithm = NA_ARIA;
        }
        else
        {
            fprintf(stderr, "Unkown algorithm '%s'\n", argv[2]);
            return -1;
        }
    }
    else
    {
        PrintUsage();
        return -1;
    }

    if ( argv[3] != NULL )
    {
        if ( strcmp(argv[3], "-ecb") == 0 )
        {
            pstArgs->eMode = NM_ECB;
        }
        else if ( strcmp(argv[3], "-cbc") == 0 )
        {
            pstArgs->eMode = NM_CBC;
        }
        else
        {
            fprintf(stderr, "Unkown mode '%s'\n", argv[3]);
            return -1;
        }
    }
    else
    {
        PrintUsage();
        return -1;
    }

    if ( argv[4] != NULL )
    {
        if ( strcmp(argv[4], "-null") == 0 )
        {
            pstArgs->ePadding = NP_NULL;
        }
        else if ( strcmp(argv[4], "-pkcs") == 0 )
        {
            pstArgs->ePadding = NP_PKCS;
        }
        else
        {
            fprintf(stderr, "Unkown padding '%s'\n", argv[4]);
            return -1;
        }
    }
    else
    {
        PrintUsage();
        return -1;
    }

    if ( argv[5] != NULL )
    {
        pstArgs->pKey  = argv[5];
    }
    else
    {
        PrintUsage();
        return -1;
    }

    if ( argv[6] != NULL )
    {
        pstArgs->pInStream  = argv[6];
        pstArgs->szInStream = strlen(argv[6]);
    }
    else
    {
        PrintUsage();
        return -1;
    }

    return 0;
}

static void PrintUsage()
{
    fprintf(stdout, "\n");
    fprintf(stdout, "> 'ncrypt' Usage : ncrypt [-worktype] [-algorithm] [-mode] [-padding] [key] [string]\n");
    fprintf(stdout, " \n[worktype]\n");
    fprintf(stdout, "  1. -e : Encrypt.\n");
    fprintf(stdout, "  2. -d : Decrypt.\n");
    fprintf(stdout, " \n[algorithm]\n");
    fprintf(stdout, "  1. -seed  : KISA-SEED algorithm.\n");
    fprintf(stdout, "  2. -hight : KISA-HIGHT algorithm.\n");
    fprintf(stdout, "  3. -aria  : KISA-ARIA algorithm.\n");
    fprintf(stdout, " \n[mode]\n");
    fprintf(stdout, "  1. -ecb : ECB(eletronic code book) mode.\n");
    fprintf(stdout, "  2. -cbc : CBC(cipher block chaning) mode.\n");
    fprintf(stdout, " \n[padding]\n");
    fprintf(stdout, "  1. -null : null padding.\n");
    fprintf(stdout, "  2. -pkcs : pkcs padding.\n");
    fprintf(stdout, "\n");
}

int main(int argc, char **argv)
{
    NcryptArgs stArgs;
    
    if ( CheckArgs(argc, argv, &stArgs) != 0 )
    {
        return -1;
    }

    if ( NcryptorTest(&stArgs) != 0 )
    {
        return -1;
    }

    return 0;
}