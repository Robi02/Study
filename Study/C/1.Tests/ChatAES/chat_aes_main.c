#include "common_include.h"
#include "common_typedef.h"
#include "lib_ncryptor.h"
#include "lib_ncoder.h"

static BYTE gAB_KEY32[32] = "0KsNeTkSnEtKsNeTkSnEtKsNeTkSnEt0";
static BYTE gAB_IV16[16]  = "010kSnEtKsNeT010";
static const int  gSZ_BUF = 1024;

int main()
{
    NCRYPT_ALGO_AES_ARGS   stAES;
    NCODE_TYPE_BASE64_ARGS stBase64;
    BYTE abIn[gSZ_BUF * 2];
    BYTE abOu[gSZ_BUF * 2];
    BYTE abRst[gSZ_BUF * 2];
    BYTE *pbByte = NULL;
    int  rtVal;

    while (1)
    {
        // Init
        memset(abIn,      0x00, sizeof(abIn));
        memset(abOu,      0x00, sizeof(abOu));
        memset(abRst,     0x00, sizeof(abRst));
        memset(&stAES,    0x00, sizeof(stAES));
        memset(&stBase64, 0x00, sizeof(stBase64));

        // Input Plane or Cipher text
        fprintf(stdout, "\n> Input: ");
        if ((pbByte = fgets(abIn, gSZ_BUF, stdin)) == NULL)
        {
            continue;
        }

        if ((pbByte = strchr(abIn, '\n')) == NULL)
        {
            continue;
        }

        *pbByte = '\0';

        // Encryption
        stAES.eKeyBit = NCRYPT_AES_KEYBIT_256;
        stAES.stArgs.isEncrypt = 1;
        stAES.stArgs.eAlgo = NCRYPT_ALGO_AES;
        stAES.stArgs.ePadd = NCRYPT_PADD_PKCS;
        stAES.stArgs.eMode = NCRYPT_MODE_CBC;
        stAES.stArgs.pbIV  = gAB_IV16;
        stAES.stArgs.szIV  = sizeof(gAB_IV16);
        stAES.stArgs.pbKey = gAB_KEY32;
        stAES.stArgs.szKey = sizeof(gAB_KEY32);
        stAES.stArgs.pbInStream = abIn;
        stAES.stArgs.szInStream = strlen(abIn);
        stAES.stArgs.pbOutBuf = abOu;
        stAES.stArgs.szOutBuf = sizeof(abOu);

        if ((rtVal = Ncrypt(&stAES, sizeof(stAES))) < 0)
        {
            fprintf(stderr, "Ncrypt() error. (return:%d)\n", rtVal);
            continue;
        }

        stBase64.isEncode   = 1;
        stBase64.pbInStream = abOu;
        stBase64.szInStream = rtVal;
        stBase64.pbOutBuf   = abRst;
        stBase64.szOutBuf   = sizeof(abRst);

        if ((rtVal = Ncode(NCODE_TYPE_BASE64, &stBase64, sizeof(stBase64))) < 0)
        {
            fprintf(stderr, "Ncode() error. (return:%d)\n", rtVal);
            continue;
        }

        fprintf(stdout, "\n> EncryptedMsg:\n");
        PrintStreamToString(abRst, rtVal);

        // Decryption
        memset(abOu,      0x00, sizeof(abOu));
        memset(abRst,     0x00, sizeof(abRst));
        memset(&stAES,    0x00, sizeof(stAES));
        memset(&stBase64, 0x00, sizeof(stBase64));

        stBase64.isEncode   = 0;
        stBase64.pbInStream = abIn;
        stBase64.szInStream = strlen(abIn);
        stBase64.pbOutBuf   = abOu;
        stBase64.szOutBuf   = sizeof(abOu);

        if (stBase64.szInStream % 4 != 0)
        {
            continue;
        }

        if ((rtVal = Ncode(NCODE_TYPE_BASE64, &stBase64, sizeof(stBase64))) < 0)
        {
            fprintf(stderr, "Ncode() error. (return:%d)\n", rtVal);
            continue;
        }

        stAES.eKeyBit = NCRYPT_AES_KEYBIT_256;
        stAES.stArgs.isEncrypt = 0;
        stAES.stArgs.eAlgo = NCRYPT_ALGO_AES;
        stAES.stArgs.ePadd = NCRYPT_PADD_PKCS;
        stAES.stArgs.eMode = NCRYPT_MODE_CBC;
        stAES.stArgs.pbIV  = gAB_IV16;
        stAES.stArgs.szIV  = sizeof(gAB_IV16);
        stAES.stArgs.pbKey = gAB_KEY32;
        stAES.stArgs.szKey = sizeof(gAB_KEY32);
        stAES.stArgs.pbInStream = abOu;
        stAES.stArgs.szInStream = strlen(abOu);
        stAES.stArgs.pbOutBuf = abRst;
        stAES.stArgs.szOutBuf = sizeof(abRst);

        if ((rtVal = Ncrypt(&stAES, sizeof(stAES))) < 0)
        {
            fprintf(stderr, "Ncrypt() error. (return:%d)\n", rtVal);
            continue;
        }

        fprintf(stdout, "\n> DecryptedMsg:\n");
        PrintStreamToString(abRst, rtVal);

        fprintf(stdout, "\n================================================================\n");
    }

    return 0;
}