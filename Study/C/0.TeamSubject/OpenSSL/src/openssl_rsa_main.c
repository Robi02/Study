#include "openssl_rsa_main.h"
#include <openssl/err.h>

#define SZ_BUF_MAX 512 /* Theoretical max buf size : 256 * 4 / 3 = 342 */
#define RSA_PUBLIC_PEM_NAME "./public.pem"
#define RSA_PRIVATE_PEM_NAME "./private.pem"

int Generate_RSA_Key(int rsaBit)
{
    int             ret = 0;
    RSA             *rsa = NULL;
    BIGNUM          *bne = NULL;
    BIO             *bp_public = NULL, *bp_private = NULL;
    unsigned long   e = RSA_F4;
 
    // 1. Generate RSA private key
    bne = BN_new();
    ret = BN_set_word(bne, e);
    if (ret != 1) { fprintf(stdout, "RSA public key set error!\n"); goto END; }
 
    rsa = RSA_new();
    ret = RSA_generate_key_ex(rsa, rsaBit, bne, NULL);
    if (ret != 1) { fprintf(stdout, "RSA private key generate error!\n"); goto END; }
 
    // 2. Save RSA public key
    bp_public = BIO_new_file(RSA_PUBLIC_PEM_NAME, "w+");
    ret = PEM_write_bio_RSAPublicKey(bp_public, rsa);
    if (ret != 1) { fprintf(stdout, "RSA public key save error!\n"); goto END; }

    // 3. Save RSA private key
    bp_private = BIO_new_file(RSA_PRIVATE_PEM_NAME, "w+");
    ret = PEM_write_bio_RSAPrivateKey(bp_private, rsa, NULL, NULL, 0, NULL, NULL);
    if (ret != 1) { fprintf(stdout, "RSA private key save error!\n"); goto END; }
 
END:
    /* 4. Free temporal memory */
    BIO_free_all(bp_public);
    BIO_free_all(bp_private);
    RSA_free(rsa);
    BN_free(bne);
    return ret;
}

int Encrypt_RSA(unsigned char *pPlainText, size_t szPlainText, unsigned char *pCipherText)
{
    int                     szTempBuf = SZ_BUF_MAX * 2;
    unsigned char           tempBuf[szTempBuf];
    NCODE_TYPE_BASE64_ARGS  stB64Args;
    int                     ret = 0, len = -1;
    RSA                     *rsa = NULL;
    BIO                     *bp_public = NULL;
    
    /* 1. Open PEM public key file to BIO */
    bp_public = BIO_new(BIO_s_file());
    ret = BIO_read_filename(bp_public, RSA_PUBLIC_PEM_NAME);
    if (ret != 1) { fprintf(stdout, "RSA public key bio open error!\n"); goto END; }

    /* 2. Encrypt with public key */
    PEM_read_bio_RSAPublicKey(bp_public, &rsa, NULL, NULL);
    len = RSA_public_encrypt(szPlainText, pPlainText, pCipherText, rsa, RSA_PKCS1_PADDING);
    if (len == -1) { fprintf(stdout, "RSA_public_encrypt error!\n"); goto END; }

    /* 3. Encode binary cipher to Base64 text */
    stB64Args.isEncode   = 1;
    stB64Args.pbInStream = pCipherText;
    stB64Args.szInStream = len;
    stB64Args.pbOutBuf   = tempBuf;
    stB64Args.szOutBuf   = szTempBuf;
    len = Ncode(NCODE_TYPE_BASE64, &stB64Args, sizeof(stB64Args));
    memset(pCipherText, 0x00, szTempBuf);
    memcpy(pCipherText, tempBuf, len);

END:
    /* 4. Free temporal memory */
    BIO_free_all(bp_public);
    RSA_free(rsa);
    return len;
}

int Decrypt_RSA(unsigned char *pCipherText, size_t szCipherText, unsigned char *pPlainText)
{
    int                     szTempBuf = SZ_BUF_MAX * 2;
    unsigned char           tempBuf[szTempBuf];
    NCODE_TYPE_BASE64_ARGS  stB64Args;
    int                     ret = 0, len = -1;
    RSA                     *rsa = NULL;
    BIO                     *bp_private = NULL;

    /* 1. Decode Base64 text to binary cipher */
    stB64Args.isEncode   = 0;
    stB64Args.pbInStream = pCipherText;
    stB64Args.szInStream = szCipherText;
    stB64Args.pbOutBuf   = tempBuf;
    stB64Args.szOutBuf   = szTempBuf;
    len = Ncode(NCODE_TYPE_BASE64, &stB64Args, sizeof(stB64Args));

    /* 2. Open PEM private key to BIO */
    bp_private = BIO_new(BIO_s_file());
    ret = BIO_read_filename(bp_private, RSA_PRIVATE_PEM_NAME);
    if (ret != 1) { fprintf(stdout, "RSA private key bio open error!\n"); goto END; }

    /* 3. Decrypt with private key */
    PEM_read_bio_RSAPrivateKey(bp_private, &rsa, NULL, NULL);
    len = RSA_private_decrypt(len, tempBuf, pPlainText, rsa, RSA_PKCS1_PADDING);
    if (len == -1) { fprintf(stdout, "RSA_public_decrypt error!\n"); goto END; }

END:
    /* 4. Free temporal memory */
    BIO_free_all(bp_private);
    RSA_free(rsa);
    return len;
}

void PrintUsage()
{
    fprintf(stdout, "[rsacrypt Usage]\n");
    fprintf(stdout, "rsacrypt.exe [Op] [Bit] [String]\n");
    fprintf(stdout, " [Op]\n");
    fprintf(stdout, "  1. -e : Encryption.\n");
    fprintf(stdout, "  2. -d : Decryption.\n");
    fprintf(stdout, " [Bit]\n");
    fprintf(stdout, "  1. -1024 : RSA-1024.\n");
    fprintf(stdout, "  2. -2048 : RSA-2048.\n");
}

int main(int argc, char **argv)
{
    int             rtlen = 0, keyBits = 0, maxlen = 0;
    unsigned char   ops[3], bits[6];
    unsigned char   bufPlainText[SZ_BUF_MAX], bufCipherText[SZ_BUF_MAX];

    if (argc != 4) { PrintUsage(); return -1; }

    memset(bufPlainText, 0x00, sizeof(bufPlainText));
    memset(bufCipherText, 0x00, sizeof(bufCipherText));
    memset(ops, 0x00, sizeof(ops));
    memset(bits, 0x00, sizeof(bits));
    memcpy(ops, argv[1], 3);    /* [Op] */
    memcpy(bits, argv[2], 6);   /* [Bits] */

    /* [ PKCS1(v1.5) Encoded Message(EM) ]
     { EM = 0x00 || 0x02 || PS || 0x00 || Msg }
    1. EM = 256byte(2048bit), PS = 8byte + 3(0x00, 0x02, 0x00), Max msg = 245byte
    2. EM = 128byte(1024bit), PS = 8byte + 3(0x00, 0x02, 0x00), Max msg = 117byte */
    keyBits = (strcmp(bits, "-1024") == 0 ? 1024 : 2048);
    maxlen  = (keyBits == 1024 ? 117 : 245);

    if (strcmp(ops, "-e") == 0)
    {
        /* Encryption */
        memcpy(bufPlainText, argv[3], strlen(argv[3])); /* [(Plain)String] */
        fprintf(stdout, "\n> InputPlainText : [%s]\n", bufPlainText);

        Generate_RSA_Key(keyBits);
        if (rtlen == -1) { fprintf(stdout, "Error has occurred while 'Generate_RSA_Key()'!\n"); return -1; }

        rtlen = Encrypt_RSA(bufPlainText, min(strlen(bufPlainText), maxlen), bufCipherText);
        if (rtlen == -1) { fprintf(stdout, "Error has occurred while 'Encrypt_RSA()'!\n"); return -1; }

        fprintf(stdout, "\n> CipherText : [%s]\n", bufCipherText);
        fprintf(stdout, "\n - Public key PEM '%s' saved.\n", RSA_PUBLIC_PEM_NAME);
        fprintf(stdout, "\n - Private key PEM '%s' saved.\n", RSA_PRIVATE_PEM_NAME);
    }
    else
    {
        /* Decryption */
        memcpy(bufCipherText, argv[3], strlen(argv[3])); /* [(Cipher)String] */
        fprintf(stdout, "\n> InputCipherText : [%s]\n", bufCipherText);

        rtlen = Decrypt_RSA(bufCipherText, strlen(bufCipherText), bufPlainText);
        if (rtlen == -1) { fprintf(stdout, "Error has occurred while 'Decrypt_RSA()'!\n"); return -1; }
        
        fprintf(stdout, "\n> DecryptText : [%s]\n", bufPlainText);
    }    

    return 0;
}