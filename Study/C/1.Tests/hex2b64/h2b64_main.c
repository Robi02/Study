#include "common_include.h"
#include "common_typedef.h"
#include "lib_ncoder.h"

#define SZ_BUF 2048

int main()
{
    NCODE_TYPE_HEXA_ARGS stHexa;
    NCODE_TYPE_BASE64_ARGS stBase64;
    BYTE *pbByte = NULL;
    BYTE abBuf[SZ_BUF];
    int rtVal;

    while (1)
    {
        // Init        
        memset(&stHexa,   0x00, sizeof(stHexa));
        memset(&stBase64, 0x00, sizeof(stBase64));
        memset(&abBuf,    0x00, sizeof(abBuf));

        // Input HEXA
        fprintf(stdout, "\n> Input Hexa: ");
        if ((pbByte = fgets(abBuf, SZ_BUF, stdin)) == NULL)
        {
            continue;
        }

        if ((pbByte = strchr(abBuf, '\n')) == NULL)
        {
            continue;
        }

        *pbByte = '\0';

        // Hexa to binary
        int szInBufHexa = strlen(abBuf);
        BYTE abInBufHexa[szInBufHexa];
        int szOuBufHexa = szInBufHexa * 2;
        BYTE abOuBufHexa[szOuBufHexa];

        memcpy(abInBufHexa, abBuf, szInBufHexa);
        memset(abOuBufHexa, 0x00, sizeof(abOuBufHexa));

        stHexa.isEncode   = 0;
        stHexa.pbInStream = abInBufHexa;
        stHexa.szInStream = szInBufHexa;
        stHexa.pbOutBuf   = abOuBufHexa;
        stHexa.szOutBuf   = szOuBufHexa;

        if ((rtVal = Ncode(NCODE_TYPE_HEXA, &stHexa, sizeof(stHexa))) < 0)
        {
            fprintf(stderr, "Ncode(Hex2Bin) error. (return:%d)\n", rtVal);
            continue;
        }

        int szInBufBase64 = rtVal;
        BYTE abInBufBase64[szInBufBase64];
        int szOuBufBase64 = szInBufBase64 * 2;
        BYTE abOuBufBase64[szOuBufBase64];
        
        memcpy(abInBufBase64, abOuBufHexa, rtVal);
        memset(abOuBufBase64, 0x00, sizeof(abOuBufBase64));

        // Binary to Base64
        stBase64.isEncode   = 1;
        stBase64.pbInStream = abInBufBase64;
        stBase64.szInStream = szInBufBase64;
        stBase64.pbOutBuf   = abOuBufBase64;
        stBase64.szOutBuf   = szOuBufBase64;

        if ((rtVal = Ncode(NCODE_TYPE_BASE64, &stBase64, sizeof(stBase64))) < 0)
        {
            fprintf(stderr, "Ncode(Bin2Base64) error. (return:%d)\n", rtVal);
            continue;
        }

        fprintf(stdout, "\n> Input Hexa String:\n");
        PrintStreamToString(abInBufHexa, sizeof(abInBufHexa));

        fprintf(stdout, "\n> Cvt Base64:\n");
        PrintStreamToString(abOuBufBase64, rtVal);

        fprintf(stdout, "\n> Cvt Base64 (Print Hexa):\n");
        PrintStreamToHexa(abOuBufBase64, rtVal);

        fprintf(stdout, "\n================================================================\n");
    }

    return 0;
}