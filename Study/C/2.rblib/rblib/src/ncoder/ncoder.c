#include "common_include.h"
#include "ncode_hexa.h"
#include "ncode_base64.h"
#include "ncoder.h"

int Ncode(NCODE_TYPE eNcodeType, void *pstArgs, int szArgs)
{
    if (eNcodeType == NCODE_TYPE_HEXA)
    {
        if (sizeof(NCODE_TYPE_HEXA_ARGS) != szArgs)
        {
            fprintf(stderr, "sizeof(NCODE_TYPE_HEXA_ARGS) != szArgs (%ld != %d)\n", sizeof(NCODE_TYPE_HEXA_ARGS), szArgs);
            return -1;
        }

        return ((NCODE_TYPE_HEXA_ARGS *)pstArgs)->isEncode == 1 ? Encode_Hexa(pstArgs) : Decode_Hexa(pstArgs);
    }
    else if (eNcodeType == NCODE_TYPE_BASE64)
    {
        if (sizeof(NCODE_TYPE_BASE64_ARGS) != szArgs)
        {
            fprintf(stderr, "sizeof(NCODE_TYPE_BASE64_ARGS) != szArgs (%ld != %d)\n", sizeof(NCODE_TYPE_BASE64_ARGS), szArgs);
            return -1;
        }

        return ((NCODE_TYPE_BASE64_ARGS *)pstArgs)->isEncode == 1 ? Encode_Base64(pstArgs) : Decode_Base64(pstArgs);
    }
    else if (0)
    {
        // Add new NCODE_TYPE here...
        return -1;
    }
    else
    {
        fprintf(stderr, "Unknown eNcodeType. (eNcodeType:%d)\n", eNcodeType);
        return -1;
    }
}

static int ncoder_test(int argc, char **argv)
{    
    NCODE_TYPE eNcodeType;
    void *pArgs;
    int szArgs, isEncoding, rtVal;
    BYTE abInBuf[2048], abOutBuf[2048];

    if (argc != 4)
    {
        fprintf(stdout, "> Ncoder Usage : ncoder [-worktype] [-algorithm] [string]\n\n");
        fprintf(stdout, "[-worktype]\n");
        fprintf(stdout, " 1. -e : Encoding work.\n");
        fprintf(stdout, " 2. -d : Decoding work.\n");
        fprintf(stdout, "[-algorithm]\n");
        fprintf(stdout, " 1. -h   : hexa converting.\n");
        fprintf(stdout, " 2. -b64 : base64 converting.\n");
        fprintf(stdout, "\n");
        return -1;
    }
    
    memset(abInBuf, 0x00, sizeof(abInBuf));
    memcpy(abInBuf, argv[3], strlen(argv[3]));
    memset(abOutBuf, 0x00, sizeof(abOutBuf));

    isEncoding = (strcmp(argv[1], "-e") == 0 ? 1 : 0);

    if (strcmp(argv[2], "-h") == 0)
    {
        NCODE_TYPE_HEXA_ARGS stArgsHexa;

        eNcodeType = NCODE_TYPE_HEXA;
        szArgs = sizeof(NCODE_TYPE_HEXA_ARGS);

        stArgsHexa.isEncode    = isEncoding;
        stArgsHexa.isLowercase = 0;
        stArgsHexa.pbInStream  = abInBuf;
        stArgsHexa.pbOutBuf    = abOutBuf;
        stArgsHexa.szInStream  = strlen(argv[3]);
        stArgsHexa.szOutBuf    = sizeof(abOutBuf);
        
        pArgs = (void *)&stArgsHexa;
        fprintf(stdout, "inArgs : [isEncode:%d, pbInStream:%s, szInStream:%d szOutBuf:%d]\n",
                stArgsHexa.isEncode, stArgsHexa.pbInStream, stArgsHexa.szInStream, stArgsHexa.szOutBuf);
    }
    else if (strcmp(argv[2], "-b64") == 0)
    {
        NCODE_TYPE_BASE64_ARGS stArgsBase64;
        
        eNcodeType = NCODE_TYPE_BASE64;
        szArgs = sizeof(NCODE_TYPE_BASE64_ARGS);

        stArgsBase64.isEncode   = isEncoding;
        stArgsBase64.pbInStream = abInBuf;
        stArgsBase64.szInStream = strlen(abInBuf);
        stArgsBase64.pbOutBuf   = abOutBuf;
        stArgsBase64.szOutBuf   = sizeof(abOutBuf);

        pArgs = (void *)&stArgsBase64;
        fprintf(stdout, "inArgs : [isEncode:%d, pbInStream:%s, szInStream:%d szOutBuf:%d]\n",
                stArgsBase64.isEncode, stArgsBase64.pbInStream, stArgsBase64.szInStream, stArgsBase64.szOutBuf);
    }
    else if (0)
    {
        // ...
    }
    else
    {
        fprintf(stderr, "Error! Unkown ncode type '%s'.\n", argv[2]);
        return -1;
    }

    if ((rtVal = Ncode(eNcodeType, (void *)pArgs, szArgs)) <= 0)
    {
        fprintf(stderr, "Ncode() error. (return:%d)\n", rtVal);
        return -1;
    }

    fprintf(stdout, "outSteram : [%s] (rtVal:%d)\n", abOutBuf, rtVal);

    return 0;
}

//int main(int argc, char **argv)
//{
//  return ncoder_test(argc, argv);
//}