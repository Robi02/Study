#include "openssl_block_main.h"

#define SZ_IV   16
#define SZ_KEY  128
#define SZ_BUF  1024
#define SZ_BUF2 2048

static int Crypto_DES(int nOp, int nMode, BYTE *pbPln, int szPln, BYTE *pbCip, int szCip, 
                      BYTE *pbB64, int szB64, BYTE *pbKey, int szKey, BYTE *pbIV, int szIV)
{
    /* Block Var */
    const int               szBlock = 8;
    int                     isTripleDES, i, rtVal;
    const_DES_cblock        abInBlock;
    DES_cblock              abOuBlock, abIvBlock;
    NCODE_TYPE_BASE64_ARGS  stB64Args;

    /* Key Var */
    const_DES_cblock abKeyBlock, abKeyBlock2, abKeyBlock3;
    DES_key_schedule stDesKey, stDesKey2, stDesKey3;

    /* Init */
    memset(  &stDesKey, 0x00, sizeof(stDesKey)   );
    memset( &stDesKey2, 0x00, sizeof(stDesKey2)  );
    memset( &stDesKey3, 0x00, sizeof(stDesKey3)  );
    memset( abKeyBlock, 0x00, sizeof(abKeyBlock) );
    memset(abKeyBlock2, 0x00, sizeof(abKeyBlock2));
    memset(abKeyBlock3, 0x00, sizeof(abKeyBlock3));
    memset(  abInBlock, 0x00, sizeof(abInBlock)  );
    memset(  abOuBlock, 0x00, sizeof(abOuBlock)  );
    memset(  abIvBlock, 0x00, sizeof(abIvBlock)  );
    memset( &stB64Args, 0x00, sizeof(stB64Args)  );

    /* Key Setting */
    memcpy(abKeyBlock, &pbKey[0], sizeof(abKeyBlock));
    DES_set_key(&abKeyBlock, &stDesKey); /* Key1 */

    if (szKey == 16 || szKey == 24)
    {
        isTripleDES = 1;
        memcpy(abKeyBlock2, &pbKey[8], sizeof(abKeyBlock2));
        DES_set_key(&abKeyBlock2, &stDesKey2); /* Key2 */

        if (szKey == 16)
        {
            memcpy(abKeyBlock3, &pbKey[0], sizeof(abKeyBlock3));
            DES_set_key(&abKeyBlock3, &stDesKey3); /* Key3 */
        }
        else
        {
            memcpy(abKeyBlock2, &pbKey[16], sizeof(abKeyBlock3));
            DES_set_key(&abKeyBlock3, &stDesKey3); /* Key3 */
        }
    }
    else { isTripleDES = 0; }

    if (nOp == 0) /* Encryption */
    {
        /* Encrypt */
        if (nMode == 0) /* ECB */
        {
            if (isTripleDES == 0) /* Single DES */
            {
                for (i = 0; i < szPln; i += szBlock)
                { 
                    memcpy(abInBlock, &pbPln[i], szBlock);
                    DES_ecb_encrypt(&abInBlock, &abOuBlock, &stDesKey, DES_ENCRYPT);
                    memcpy(&pbCip[i], abOuBlock, szBlock);
                }
            }
            else /* Triple DES (Enc -> Dec -> Enc) */
            {
                for (i = 0; i < szPln; i += szBlock)
                {
                    memcpy(abInBlock, &pbPln[i], szBlock);
                    DES_ecb3_encrypt(&abInBlock, &abOuBlock, &stDesKey, &stDesKey2, &stDesKey3, DES_ENCRYPT);
                    memcpy(&pbCip[i], abOuBlock, szBlock);
                }
            }
        }
        else if (nMode == 1) /* CBC */
        {
            if (isTripleDES == 0) /* Single DES */
            {
                memcpy(abIvBlock, pbIV, sizeof(abIvBlock));
                DES_ncbc_encrypt(pbPln, pbCip, szPln, &stDesKey, &abIvBlock, DES_ENCRYPT);
            }
            else /* Triple DES (Enc -> Dec -> Enc) */
            {
                memcpy(abIvBlock, pbIV, sizeof(abIvBlock));
                DES_ede3_cbc_encrypt(pbPln, pbCip, szPln, &stDesKey, &stDesKey2, &stDesKey3, &abIvBlock, DES_ENCRYPT);
            }
        }

        /* Binary to Base64 */
        stB64Args.isEncode   = 1;
        stB64Args.pbInStream = pbCip;
        stB64Args.szInStream = szPln;
        stB64Args.pbOutBuf   = pbB64;
        stB64Args.szOutBuf   = szB64;

        return Ncode(NCODE_TYPE_BASE64, &stB64Args, sizeof(stB64Args));
    }
    else if (nOp == 1) /* Decryption */
    {
        /* Base64 to Binary */
        stB64Args.isEncode   = 0;
        stB64Args.pbInStream = pbB64;
        stB64Args.szInStream = szB64;
        stB64Args.pbOutBuf   = pbCip;
        stB64Args.szOutBuf   = szCip;

        rtVal = Ncode(NCODE_TYPE_BASE64, &stB64Args, sizeof(stB64Args));

        /* Decrypt */
        if (nMode == 0) /* ECB */
        {
            if (isTripleDES == 0) /* Single DES */
            {
                for (i = 0; i < rtVal; i += szBlock)
                { 
                    memcpy(abInBlock, &pbCip[i], szBlock);
                    DES_ecb_encrypt(&abInBlock, &abOuBlock, &stDesKey, DES_DECRYPT);
                    memcpy(&pbPln[i], abOuBlock, szBlock);
                }
            }
            else /* Triple DES (Dec -> Enc -> Dec) */
            {
                for (i = 0; i < rtVal; i += szBlock)
                {
                    memcpy(abInBlock, &pbCip[i], szBlock);
                    DES_ecb3_encrypt(&abInBlock, &abOuBlock, &stDesKey, &stDesKey2, &stDesKey3, DES_DECRYPT);
                    memcpy(&pbPln[i], abOuBlock, szBlock);
                }
            }
        }
        else if (nMode == 1) /* CBC */
        {
            if (isTripleDES == 0) /* Single DES */
            {
                memcpy(abIvBlock, pbIV, sizeof(abIvBlock));
                DES_ncbc_encrypt(pbCip, pbPln, rtVal, &stDesKey, &abIvBlock, DES_DECRYPT);
            }
            else /* Triple DES (Enc -> Dec -> Enc) */
            {
                memcpy(abIvBlock, pbIV, sizeof(abIvBlock));
                DES_ede3_cbc_encrypt(pbCip, pbPln, rtVal, &stDesKey, &stDesKey2, &stDesKey3, &abIvBlock, DES_DECRYPT);
            }
        }

        return rtVal;
    }

    return -1;
}

static int Crypto_AES(int nOp, int nMode, BYTE *pbPln, int szPln, BYTE *pbCip, int szCip, 
                      BYTE *pbB64, int szB64, BYTE *pbKey, int szKey, BYTE *pbIV, int szIV)
{
    const int szBlock = 16;
    int i, rtVal;

    if (nOp == 0)
    {
        AES_KEY stAesEncKey;
        NCODE_TYPE_BASE64_ARGS stB64Args;

        /* Init */
        memset(&stAesEncKey, 0x00, sizeof(stAesEncKey));
        memset(  &stB64Args, 0x00, sizeof(stB64Args));
        AES_set_encrypt_key(pbKey, szKey * 8, &stAesEncKey);

        /* Encrypt */
        if (nMode == 0)
        {
            for (i = 0; i < szPln; i += szBlock)
            {
                AES_ecb_encrypt(&pbPln[i], &pbCip[i], &stAesEncKey, AES_ENCRYPT);
            }
        }
        else if (nMode == 1) { AES_cbc_encrypt(pbPln, pbCip, szPln, &stAesEncKey, pbIV, AES_ENCRYPT); }

        /* Binary to Base64 */
        stB64Args.isEncode   = 1;
        stB64Args.pbInStream = pbCip;
        stB64Args.szInStream = szPln;
        stB64Args.pbOutBuf   = pbB64;
        stB64Args.szOutBuf   = szB64;

        return Ncode(NCODE_TYPE_BASE64, &stB64Args, sizeof(stB64Args));
    }
    else if (nOp == 1)
    {
        AES_KEY stAesDecKey;
        NCODE_TYPE_BASE64_ARGS stB64Args;

        /* Init */
        memset(&stAesDecKey, 0x00, sizeof(stAesDecKey));
        memset(  &stB64Args, 0x00, sizeof(stB64Args));
        AES_set_decrypt_key(pbKey, szKey * 8, &stAesDecKey);

        /* Base64 to Binary */
        stB64Args.isEncode   = 0;
        stB64Args.pbInStream = pbB64;
        stB64Args.szInStream = szB64;
        stB64Args.pbOutBuf   = pbCip;
        stB64Args.szOutBuf   = szCip;

        rtVal = Ncode(NCODE_TYPE_BASE64, &stB64Args, sizeof(stB64Args));

        /* Decrypt */
        if (nMode == 0)
        {
            for (i = 0; i < rtVal; i += 16)
            {
                AES_ecb_encrypt(&pbCip[i], &pbPln[i], &stAesDecKey, AES_DECRYPT);
            }
        }
        else if (nMode == 1) { AES_cbc_encrypt(pbCip, pbPln, rtVal, &stAesDecKey, pbIV, AES_DECRYPT); }

        return rtVal;
    }

    return -1;
}

static int Crypto_BLOWFISH(int nOp, int nMode, BYTE *pbPln, int szPln, BYTE *pbCip, int szCip, 
                           BYTE *pbB64, int szB64, BYTE *pbKey, int szKey, BYTE *pbIV, int szIV)
{
    const int szBlock = 8;
    int i, rtVal;

    if (nOp == 0)
    {
        BF_KEY stBfKey;
        NCODE_TYPE_BASE64_ARGS stB64Args;

        /* Init */
        memset(  &stBfKey, 0x00, sizeof(stBfKey)  );
        memset(&stB64Args, 0x00, sizeof(stB64Args));
        BF_set_key(&stBfKey, szKey, pbKey);

        /* Encrypt */
        if (nMode == 0)
        {
            for (i = 0; i < szPln; i += 8)
            {
                BF_ecb_encrypt(&pbPln[i], &pbCip[i], &stBfKey, BF_ENCRYPT);
            }
        }
        else if (nMode == 1) { BF_cbc_encrypt(pbPln, pbCip, szPln, &stBfKey, pbIV, BF_ENCRYPT); }

        /* Binary to Base64 */
        stB64Args.isEncode   = 1;
        stB64Args.pbInStream = pbCip;
        stB64Args.szInStream = szPln;
        stB64Args.pbOutBuf   = pbB64;
        stB64Args.szOutBuf   = szB64;

        return Ncode(NCODE_TYPE_BASE64, &stB64Args, sizeof(stB64Args));
    }
    else if (nOp == 1)
    {
        BF_KEY stBfKey;
        NCODE_TYPE_BASE64_ARGS stB64Args;

        /* Init */
        memset(  &stBfKey, 0x00, sizeof(stBfKey)  );
        memset(&stB64Args, 0x00, sizeof(stB64Args));
        BF_set_key(&stBfKey, szKey, pbKey);

        /* Base64 to Binary */
        stB64Args.isEncode   = 0;
        stB64Args.pbInStream = pbB64;
        stB64Args.szInStream = szB64;
        stB64Args.pbOutBuf   = pbCip;
        stB64Args.szOutBuf   = szCip;

        rtVal = Ncode(NCODE_TYPE_BASE64, &stB64Args, sizeof(stB64Args));

        /* Decrypt */
        if (nMode == 0)
        {
            for (i = 0; i < rtVal; i += szBlock)
            {
                BF_ecb_encrypt(&pbCip[i], &pbPln[i], &stBfKey, BF_DECRYPT);
            }
        }
        else if (nMode == 1) { BF_cbc_encrypt(pbCip, pbPln, rtVal, &stBfKey, pbIV, BF_DECRYPT); }

        return rtVal;
    }

    return -1;
}

static int MakePadding(int nPadd, BYTE *pIn, int szInBuf, int szBlock)
{
    int  szIn, szPadd, szPaddEnd, i;
    BYTE bPaddSym;

    if (pIn == NULL) { return -1; }

    szIn   = strlen(pIn);
    szPadd = szBlock - (szIn % szBlock);

    if      (nPadd == 0) { nPadd = 0; bPaddSym = (BYTE)0x00;   } /* nPadd(0): NULL */
    else if (nPadd == 1) { nPadd = 1; bPaddSym = (BYTE)szPadd; } /* nPadd(1): PKCS */
    else                 { return -1; }

    if (nPadd == 1 && szPadd == szBlock) { szPadd = szBlock * 2; } /* PKCS, BlockFull -> Add new block */

    szPaddEnd = szIn + szPadd;

    for (i = szIn; i < szPaddEnd; ++i)
    {
        if (i > szInBuf) { return -1; }
        pIn[i] = bPaddSym;
    }

    return i;
}

static int OpenSSL_Crypt(int nOp, int nAlgo, int nMode, int nPadd, 
                         BYTE *pbPln, int szPln, BYTE *pbCip, int szCip, 
                         BYTE *pbB64, int szB64, BYTE *pbKey, int szKey, BYTE *pbIV, int szIV)
{
    int szPaddPln = 0;

    if (nAlgo == 0) /* DES */
    {
        if (nOp == 0 && (szPaddPln = MakePadding(nPadd, pbPln, SZ_BUF, 8)) < 0) { return -1; }
        return Crypto_DES(nOp, nMode, pbPln, szPaddPln, pbCip, szCip, pbB64, szB64, pbKey, szKey, pbIV, szIV);
    }
    else if (nAlgo == 1) /* AES */
    { 
        if (nOp == 0 && (szPaddPln = MakePadding(nPadd, pbPln, SZ_BUF, 16)) < 0) { return -1; }
        return Crypto_AES(nOp, nMode, pbPln, szPaddPln, pbCip, szCip, pbB64, szB64, pbKey, szKey, pbIV, szIV);
    }
    else if (nAlgo == 2) /* BLOWFISH */
    {
        if (nOp == 0 && (szPaddPln = MakePadding(nPadd, pbPln, SZ_BUF, 8)) < 0) { return -1; }
        return Crypto_BLOWFISH(nOp, nMode, pbPln, szPaddPln, pbCip, szCip, pbB64, szB64, pbKey, szKey, pbIV, szIV);
    }
    else if (0) { /* New Algo Here... */ }

    return -1;
}

static void PrintUsage()
{
    fprintf(stdout, "[Crypto Usage]                                                \n");
    fprintf(stdout, "openssl_test [-op] [-algo] [-mode] [-padd] [key] [string] (iv)\n");
    fprintf(stdout, " 1. [-op] : -e, -d                                            \n");
    fprintf(stdout, " 2. [-algo] : -des, -aes, -blowfish                           \n");
    fprintf(stdout, " 3. [-mode] : -ecb, -cbc                                      \n");
    fprintf(stdout, " 4. [-padd] : -null, -pkcs                                    \n");
}

int main(int argc, char **argv)
{
    BYTE abPln[SZ_BUF], abCip[SZ_BUF], abB64[SZ_BUF2], abKey[SZ_KEY], abIV[SZ_IV];
    int  szPln, szCip, szB64, szKey, szIV;
    int  nOp, nAlgo, nMode, nPadd;
    int  szBlock, rtVal;

    memset(abPln, 0x00, sizeof(abPln));
    memset(abCip, 0x00, sizeof(abCip));
    memset(abB64, 0x00, sizeof(abB64));
    memset(abKey, 0x00, sizeof(abKey));
    memset( abIV, 0x00, sizeof(abIV) );

    /* Check argc */
    if (argc < 7) { PrintUsage(); rtVal = -1; goto ERR; }
    
    /* Check Op */
    if      (strcmp(argv[1], "-e") == 0) { nOp = 0;              }
    else if (strcmp(argv[1], "-d") == 0) { nOp = 1;              }
    else                                 { rtVal = -2; goto ERR; }

    /* Check Algo */
    if      (strcmp(argv[2], "-des"     ) == 0) { nAlgo = 0; szBlock =  8; }
    else if (strcmp(argv[2], "-aes"     ) == 0) { nAlgo = 1; szBlock = 16; }
    else if (strcmp(argv[2], "-blowfish") == 0) { nAlgo = 2; szBlock =  8; }
    else                                        { rtVal = -3; goto ERR;    }

    /* Check Mode */
    if      (strcmp(argv[3], "-ecb") == 0) { nMode = 0;            }
    else if (strcmp(argv[3], "-cbc") == 0) { nMode = 1;            }
    else                                   { rtVal = -4; goto ERR; }

    /* Check Padd */
    if      (strcmp(argv[4], "-null") == 0) { nPadd = 0;            }
    else if (strcmp(argv[4], "-pkcs") == 0) { nPadd = 1;            }
    else                                    { rtVal = -5; goto ERR; }

    /* Copy Pln, Key, (IV) */
    if (nOp == 0)
    {
        szPln = min(strlen(argv[6]), sizeof(abPln));
        memcpy(abPln, argv[6], szPln);
        szB64 = sizeof(abB64);
    }
    else if (nOp == 1)
    {
        szPln = sizeof(abPln);
        szB64 = min(strlen(argv[6]), sizeof(abB64));
        memcpy(abB64, argv[6], szB64);
    }

    szCip = sizeof(abCip);
    szKey = min(strlen(argv[5]), sizeof(abKey));
    memcpy(abKey, argv[5], szKey);
    szIV  = 0;

    if (nMode != 0) /* nMode != ECB, copy IV */
    {
        if (argv[7] == NULL) { rtVal = -6; goto ERR; }
        szIV = min(strlen(argv[7]), szBlock);
        memcpy(abIV, argv[7], szIV);
    }

    /* En(De)cryption */
    if ((rtVal = OpenSSL_Crypt(nOp, nAlgo, nMode, nPadd, 
                               abPln, szPln, abCip, szCip, 
                               abB64, szB64, abKey, szKey, abIV, szIV)) < 1)
    {
        rtVal = -7;
        goto ERR;
    }

    /* Print Result */
    fprintf(stdout, ">> Key  (Str) : "); PrintStreamToString(abKey, szKey);
    fprintf(stdout, ">> IV   (Str) : "); PrintStreamToString(argv[7], szIV);
    fprintf(stdout, ">> Input(Str) : "); PrintStreamToString(nOp == 0 ? abPln : abB64, nOp == 0 ? szPln : szB64);
    fprintf(stdout, ">> Input(Hex) : ");   PrintStreamToHexa(nOp == 0 ? abPln : abB64, nOp == 0 ? szPln : szB64);
    fprintf(stdout, ">> Ouput(Str) : "); PrintStreamToString(nOp == 0 ? abB64 : abPln, rtVal);
    fprintf(stdout, ">> Ouput(Hex) : ");   PrintStreamToHexa(nOp == 0 ? abB64 : abPln, rtVal);

    /* Normal case */
    END:
    return rtVal;

    /* Error case */
    ERR:
    fprintf(stderr, "!Exit with error! (result:%d)\n", rtVal);
    return rtVal;
}