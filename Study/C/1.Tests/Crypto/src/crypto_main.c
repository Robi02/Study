#include "lib_common.h"
#include "lib_ncoder.h"
#include "lib_ncryptor.h"

#define SAFE_FREE_ALL(...) \
do { \
    int i = 0; \
    void *pta[] = { __VA_ARGS__ }; \
    int nVar = sizeof(pta)/sizeof(void *); \
    for(i = 0; i < nVar; ++i) \
    { \
        if (pta[i] != NULL) { free(pta[i]); } \
    } \
} while(0);

static NCRYPT_ALGO CheckAlgo(char *pArg)
{
    if (strcmp(pArg, "-des") == 0)
    {
        return NCRYPT_ALGO_DES;
    }
    else if (strcmp(pArg, "-aes") == 0)
    {
        return NCRYPT_ALGO_AES;
    }
    else if (strcmp(pArg, "-blowfish") == 0)
    {
        return NCRYPT_ALGO_BLOWFISH;
    }
    else
    {
        fprintf(stderr, "Undefined algo '%s'.\n", pArg);
        exit(-1);
    }
}

static NCRYPT_MODE CheckMode(char *pArg)
{
    if (strcmp(pArg, "-ecb") == 0)
    {
        return NCRYPT_MODE_ECB;
    }
    else if (strcmp(pArg, "-cbc") == 0)
    {
        return NCRYPT_MODE_CBC;
    }
    else
    {
        fprintf(stderr, "Undefined mode '%s'.\n", pArg);
        exit(-1);
    }
}

static NCRYPT_PADD CheckPadd(char *pArg)
{
    if (strcmp(pArg, "-null") == 0)
    {
        return NCRYPT_PADD_NULL;
    }
    else if (strcmp(pArg, "-pkcs") == 0)
    {
        return NCRYPT_PADD_PKCS;
    }
    else
    {
        fprintf(stderr, "Undefined padd '%s'.\n", pArg);
        return -1;
    }
}

static int CheckKey(NCRYPT_ALGO eAlgo, BYTE *pbKey, int szKey)
{
    if (eAlgo == NCRYPT_ALGO_DES) 
    {
        if (szKey != 8 && (szKey != 16 && szKey != 24)) // DES, TripleDES
        {
            fprintf(stderr, "DES Key size error. (szKey=%d / (Req:8,16,24)\n", szKey);
            return -1;
        }
    }
    else if (eAlgo == NCRYPT_ALGO_AES)
    {
        if (szKey != 16 && szKey != 24 && szKey != 32)
        {
            fprintf(stderr, "AES key size error. (szKey=%d / (Req:16,24,32)\n", szKey);
            return -1;
        }
    }
    else if (eAlgo == NCRYPT_ALGO_BLOWFISH)
    {
        if (szKey < 8 || szKey > 56)
        {
            fprintf(stderr, "BLOWFISH key size error. (szKey=%d / (Req:8~56)\n", szKey);
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "Undefined algo '%d'.\n", eAlgo);
        return -1;
    }

    return 0;
}

static int Encrypt(NCRYPT_ALGO eAlgo, NCRYPT_MODE eMode, NCRYPT_PADD ePadd, 
                   BYTE *pbKey, int szKey, BYTE *pbIV, int szIV, 
                   BYTE *pbPlaneMsg, int szPlaneMsg, BYTE *pbCipherMsg, int szCipherMsg,
                   BYTE *pbB64Msg, int szB64Msg)
{
    int rtVal = -2;
    NCRYPT_COMMON_BLOCK_ARGS stCom;

    // Common block arg structure init
    memset(&stCom, 0x00, sizeof(stCom));
    stCom.isEncrypt  = 1;
    stCom.eAlgo      = eAlgo;
    stCom.ePadd      = ePadd;
    stCom.eMode      = eMode;
    stCom.pbIV       = pbIV;
    stCom.szIV       = szIV;
    stCom.pbKey      = pbKey;
    stCom.szKey      = szKey;
    stCom.pbInStream = pbPlaneMsg;
    stCom.szInStream = szPlaneMsg;
    stCom.pbOutBuf   = pbCipherMsg;
    stCom.szOutBuf   = szCipherMsg;

    // Encrypt
    if (eAlgo == NCRYPT_ALGO_DES) // DES, TripleDES
    {
        NCRYPT_ALGO_DES_ARGS stDES;

        memset(&stDES, 0x00, sizeof(stDES));
        memcpy(&stDES.stArgs, &stCom, sizeof(stCom));
        
        if (szKey == 16 || szKey == 24)
        {
            stDES.isTripleDES = 1;
        }

        rtVal = Ncrypt(&stDES, sizeof(stDES));
    }
    else if (eAlgo == NCRYPT_ALGO_AES) // AES-128,196,256
    {
        NCRYPT_ALGO_AES_ARGS stAES;

        memset(&stAES, 0x00, sizeof(stAES));
        memcpy(&stAES.stArgs, &stCom, sizeof(stCom));

        if      (szKey == 16) { stAES.eKeyBit = NCRYPT_AES_KEYBIT_128; }
        else if (szKey == 24) { stAES.eKeyBit = NCRYPT_AES_KEYBIT_196; }
        else if (szKey == 32) { stAES.eKeyBit = NCRYPT_AES_KEYBIT_256; }

        rtVal = Ncrypt(&stAES, sizeof(stAES));
    }
    else if (eAlgo == NCRYPT_ALGO_BLOWFISH) // BLOWFISH
    {
        NCRYPT_ALGO_BLOWFISH_ARGS stBF;

        memset(&stBF, 0x00, sizeof(stBF));
        memcpy(&stBF.stArgs, &stCom, sizeof(stCom));

        rtVal = Ncrypt(&stBF, sizeof(stBF));
    }
    
    if (rtVal <= 0)
    {
        fprintf(stdout, "Ncrypt(Encrypt) error. (return:%d)\n", rtVal);
        return -1;
    }

    // Encode result to Base64
    NCODE_TYPE_BASE64_ARGS stBase64;

    memset(&stBase64, 0x00, sizeof(stBase64));
    stBase64.isEncode   = 1;
    stBase64.pbInStream = pbCipherMsg;
    stBase64.szInStream = rtVal;
    stBase64.pbOutBuf   = pbB64Msg;
    stBase64.szOutBuf   = szB64Msg;

    if ((rtVal = Ncode(NCODE_TYPE_BASE64, &stBase64, sizeof(stBase64))) < 0)
    {
        fprintf(stderr, "Ncode(Encode) error. (return:%d)\n", rtVal);
        return -1;
    }

    return rtVal;
}

static int Decrypt(NCRYPT_ALGO eAlgo, NCRYPT_MODE eMode, NCRYPT_PADD ePadd, 
                   BYTE *pbKey, int szKey, BYTE *pbIV, int szIV, 
                   BYTE *pbPlaneMsg, int szPlaneMsg, BYTE *pbCipherMsg, int szCipherMsg, 
                   BYTE *pbB64Msg, int szB64Msg)
{
    // Decode Base64 to binary cipher stream
    int rtVal = -2;
    NCODE_TYPE_BASE64_ARGS stBase64;

    memset(&stBase64, 0x00, sizeof(stBase64));
    stBase64.isEncode   = 0;
    stBase64.pbInStream = pbB64Msg;
    stBase64.szInStream = szB64Msg;
    stBase64.pbOutBuf   = pbCipherMsg;
    stBase64.szOutBuf   = szCipherMsg;

    if ((rtVal = Ncode(NCODE_TYPE_BASE64, &stBase64, sizeof(stBase64))) < 0)
    {
        fprintf(stderr, "Ncode(Decode) error. (return:%d)\n", rtVal);
        return -1;
    }

    // Common block arg structure init
    NCRYPT_COMMON_BLOCK_ARGS stCom;

    memset(&stCom, 0x00, sizeof(stCom));
    stCom.isEncrypt  = 0;
    stCom.eAlgo      = eAlgo;
    stCom.ePadd      = ePadd;
    stCom.eMode      = eMode;
    stCom.pbIV       = pbIV;
    stCom.szIV       = szIV;
    stCom.pbKey      = pbKey;
    stCom.szKey      = szKey;
    stCom.pbInStream = pbCipherMsg;
    stCom.szInStream = rtVal;
    stCom.pbOutBuf   = pbPlaneMsg;
    stCom.szOutBuf   = szPlaneMsg;

    // Encrypt
    if (eAlgo == NCRYPT_ALGO_DES) // DES, TripleDES
    {
        NCRYPT_ALGO_DES_ARGS stDES;

        memset(&stDES, 0x00, sizeof(stDES));
        memcpy(&stDES.stArgs, &stCom, sizeof(stCom));
        
        if (szKey == 16 || szKey == 24)
        {
            stDES.isTripleDES = 1;
        }

        rtVal = Ncrypt(&stDES, sizeof(stDES));
    }
    else if (eAlgo == NCRYPT_ALGO_AES) // AES-128,196,256
    {
        NCRYPT_ALGO_AES_ARGS stAES;

        memset(&stAES, 0x00, sizeof(stAES));
        memcpy(&stAES.stArgs, &stCom, sizeof(stCom));

        if      (szKey == 16) { stAES.eKeyBit = NCRYPT_AES_KEYBIT_128; }
        else if (szKey == 24) { stAES.eKeyBit = NCRYPT_AES_KEYBIT_196; }
        else if (szKey == 32) { stAES.eKeyBit = NCRYPT_AES_KEYBIT_256; }

        rtVal = Ncrypt(&stAES, sizeof(stAES));
    }
    else if (eAlgo == NCRYPT_ALGO_BLOWFISH) // BLOWFISH
    {
        NCRYPT_ALGO_BLOWFISH_ARGS stBF;

        memset(&stBF, 0x00, sizeof(stBF));
        memcpy(&stBF.stArgs, &stCom, sizeof(stCom));

        rtVal = Ncrypt(&stBF, sizeof(stBF));
    }
    
    if (rtVal <= 0)
    {
        fprintf(stdout, "Ncrypt(Decrypt) error. (return:%d)\n", rtVal);
        return -1;
    }

    return rtVal;
}

static void PrintUsage()
{
    fprintf(stdout, "[Crypto Usage]                                         \n");
    fprintf(stdout, "java crypto [-algo] [-mode] [-padd] [key] [string] (iv)\n");
    fprintf(stdout, " 1. [-algo] : -des, -aes, -blowfish                    \n");
    fprintf(stdout, " 2. [-mode] : -ecb, -cbc                               \n");
    fprintf(stdout, " 3. [-padd] : -null, -pkcs                             \n");
}

int main(int argc, char **argv)
{
    int rtVal = -1;

    // Check arguments
    if (argc < 6)
    {
        fprintf(stdout, "Input args error! (argc:%d < 6)\n", argc);
        PrintUsage();
        rtVal = -1;
        goto EXIT;
    }

    NCRYPT_ALGO eAlgo  = CheckAlgo(argv[1]);
    NCRYPT_MODE eMode  = CheckMode(argv[2]);
    NCRYPT_PADD ePadd  = CheckPadd(argv[3]);
    int  szKey         = strlen(argv[4]);
    BYTE *pbKey        = (BYTE *)malloc(szKey);
    int  szPlaneMsg    = strlen(argv[5]);
    int  szPlaneMsgBuf = szPlaneMsg * 2;
    BYTE *pbPlaneMsg   = (BYTE *)malloc(szPlaneMsgBuf);
    int  szIV          = 0;
    BYTE *pbIV         = NULL;
    int  szCipherMsg   = szPlaneMsgBuf;
    BYTE *pbCipherMsg  = (BYTE *)malloc(szCipherMsg);
    int  szB64Msg      = szCipherMsg * 2;
    BYTE *pbB64Msg     = (BYTE *)malloc(szB64Msg);

    if (eMode != NCRYPT_MODE_ECB)
    {
        szIV = strlen(argv[6]);
        pbIV = (BYTE *)malloc(szIV);
        memset(pbIV, 0x00, szIV);
        memcpy(pbIV, argv[6], szIV);
    }

    memset(pbPlaneMsg,  0x00, szPlaneMsgBuf);
    memset(pbKey,       0x00, szKey);
    memset(pbCipherMsg, 0x00, szCipherMsg);
    memset(pbB64Msg,    0x00, szB64Msg);

    memcpy(pbKey, argv[4], szKey);
    memcpy(pbPlaneMsg, argv[5], szPlaneMsg);

    // Key length check
    if ((rtVal = CheckKey(eAlgo, pbKey, szKey)) == -1)
    {
        fprintf(stderr, "CheckKey error. (return:%d)\n", rtVal);
        goto EXIT;
    }

    // Encryption
    fprintf(stdout, "\n>> Encrypting... (%d/%d/%d)\n", eAlgo, eMode, ePadd);
    fprintf(stdout, ">> Input : "); PrintStreamToString(pbPlaneMsg, szPlaneMsg);
    fprintf(stdout, ">> InHex : "); PrintStreamToHexa(pbPlaneMsg, szPlaneMsg);
    fprintf(stdout, ">>  Key  : "); PrintStreamToString(pbKey, szKey);
    fprintf(stdout, ">>  IV   : "); PrintStreamToString(pbIV, szIV);
    
    if ((rtVal = Encrypt(eAlgo, eMode, ePadd, pbKey, szKey, pbIV, szIV, 
                         pbPlaneMsg, szPlaneMsg, pbCipherMsg, szCipherMsg, 
                         pbB64Msg, szB64Msg)) < 1)
    {
        fprintf(stderr, "Encrypt() error. (return:%d)\n", rtVal);
        goto EXIT;
    }

    fprintf(stdout, ">> Result: "); PrintStreamToString(pbB64Msg, rtVal);
    fprintf(stdout, ">> ResHex: "); PrintStreamToHexa(pbCipherMsg, szPlaneMsg);

    // Decryption
    fprintf(stdout, "\n>> Decrypting... (%d/%d/%d)\n", eAlgo, eMode, ePadd);
    fprintf(stdout, ">> Input : "); PrintStreamToString(pbB64Msg, rtVal);
    fprintf(stdout, ">> InHex : "); PrintStreamToHexa(pbB64Msg, rtVal);
    fprintf(stdout, ">>  Key  : "); PrintStreamToString(pbKey, szKey);
    fprintf(stdout, ">>  IV   : "); PrintStreamToString(pbIV, szIV);
    
    if ((rtVal = Decrypt(eAlgo, eMode, ePadd, pbKey, szKey, pbIV, szIV, 
                         pbPlaneMsg, szPlaneMsgBuf, pbCipherMsg, szCipherMsg, 
                         pbB64Msg, rtVal)) < 1)
    {
        fprintf(stderr, "Decrypt() error. (return:%d)\n", rtVal);
        goto EXIT;
    }

    fprintf(stdout, ">> Result: "); PrintStreamToString(pbPlaneMsg, rtVal);
    fprintf(stdout, ">> ResHex: "); PrintStreamToHexa(pbPlaneMsg, rtVal);

    // Exit
EXIT:
    SAFE_FREE_ALL(pbKey, pbPlaneMsg, pbIV, pbCipherMsg, pbB64Msg);
    fprintf(stdout, "\n=¨Ï=============================================================\n");
    return rtVal;
}