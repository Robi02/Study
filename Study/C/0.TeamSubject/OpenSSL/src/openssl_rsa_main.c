#include "openssl_rsa_main.h"

#define SZ_BUF_MAX 256
#define RSA_BIT 1024
#define RSA_PUBLIC_PEM_NAME "./public.pem"
#define RSA_PRIVATE_PEM_NAME "./private.pem"

/*
[Encoded Message(EM)]
> { EM = 0x00 || 0x02 || PS || 0x00 || Msg }
1. EM = 256byte(2048bit), PS = 8byte, Max msg = 245byte
2. EM = 128byte(1024bit), PS = 8byte, Max msg = 120byte
*/

int Generate_RSA_Key()
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
    ret = RSA_generate_key_ex(rsa, RSA_BIT, bne, NULL);
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
    unsigned char           tempBuf[SZ_BUF_MAX];
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

    /* 3. Encode binary cipher to Base64 text */
    stB64Args.isEncode   = 1;
    stB64Args.pbInStream = pCipherText;
    stB64Args.szInStream = len;
    stB64Args.pbOutBuf   = tempBuf;
    stB64Args.szOutBuf   = SZ_BUF_MAX;
    len = Ncode(NCODE_TYPE_BASE64, &stB64Args, sizeof(stB64Args));
    memset(pCipherText, 0x00, SZ_BUF_MAX);
    memcpy(pCipherText, tempBuf, len);

END:
    /* 4. Free temporal memory */
    BIO_free_all(bp_public);
    RSA_free(rsa);
    return len;
}

int Decrypt_RSA(unsigned char *pCipherText, size_t szCipherText, unsigned char *pPlainText)
{
    unsigned char           tempBuf[SZ_BUF_MAX];
    NCODE_TYPE_BASE64_ARGS  stB64Args;
    int                     ret = 0, len = -1;
    RSA                     *rsa = NULL;
    BIO                     *bp_private = NULL;

    /* 1. Decode Base64 text to binary cipher */
    stB64Args.isEncode   = 0;
    stB64Args.pbInStream = pCipherText;
    stB64Args.szInStream = szCipherText;
    stB64Args.pbOutBuf   = tempBuf;
    stB64Args.szOutBuf   = SZ_BUF_MAX;
    len = Ncode(NCODE_TYPE_BASE64, &stB64Args, sizeof(stB64Args));

    /* 2. Open PEM private key to BIO */
    bp_private = BIO_new(BIO_s_file());
    ret = BIO_read_filename(bp_private, RSA_PRIVATE_PEM_NAME);
    if (ret != 1) { fprintf(stdout, "RSA private key bio open error!\n"); goto END; }

    /* 3. Decrypt with private key */
    PEM_read_bio_RSAPrivateKey(bp_private, &rsa, NULL, NULL);
    len = RSA_private_decrypt(len, tempBuf, pPlainText, rsa, RSA_PKCS1_PADDING);

END:
    /* 4. Free temporal memory */
    BIO_free_all(bp_private);
    RSA_free(rsa);
    return len;
}

int main(int argc, char **argv)
{
    int len = 0;
    unsigned char *TEST = "Hello World of RSA!";
    unsigned char bufPlainText[SZ_BUF_MAX], bufCipherText[SZ_BUF_MAX];

    memset(bufPlainText, 0x00, SZ_BUF_MAX);
    memcpy(bufPlainText, TEST, strlen(TEST)); // test

    fprintf(stdout, "> PlainText : %s\n", bufPlainText);
    Generate_RSA_Key();

    memset(bufCipherText, 0x00, SZ_BUF_MAX);
    len = Encrypt_RSA(bufPlainText, strlen(bufPlainText), bufCipherText);

    fprintf(stdout, "> CipherText : %s\n", bufCipherText);

    memset(bufPlainText, 0x00, SZ_BUF_MAX);
    len = Decrypt_RSA(bufCipherText, len, bufPlainText);

    fprintf(stdout, "> DecryptText : %s\n", bufPlainText);

    return 0;
}