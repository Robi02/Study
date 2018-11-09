#include "openssl_hash_main.h"

int Hash_SHA(unsigned char *pInStr, size_t szInStr, unsigned char *pOuBuf)
{
    int                     rtlen = -1;
    unsigned char           tempBuf[SHA256_DIGEST_LENGTH];
    NCODE_TYPE_BASE64_ARGS  args;

    memset(tempBuf, 0x00, SHA256_DIGEST_LENGTH);

    if (SHA256(pInStr, szInStr, tempBuf) == NULL) { fprintf(stdout, "SHA256() error.\n"); return -1; }

    args.isEncode   = 1;
    args.pbInStream  = tempBuf;
    args.szInStream = SHA256_DIGEST_LENGTH;
    args.pbOutBuf   = pOuBuf;
    args.szOutBuf   = SHA256_DIGEST_LENGTH * 2;
    rtlen = Ncode(NCODE_TYPE_BASE64, &args, sizeof(args));
    if (rtlen == -1) { fprintf(stdout, "Ncode() error.\n"); return -1; }

    return rtlen;
}

int Hash_md5(unsigned char *pInStr, size_t szInStr, unsigned char *pOuBuf)
{

}

int main(int argc, char **argv)
{
    // 138B8FA36F4854CAD2F1CF2AB39029718C998A108742A4B259F0C1B93E5F81DF
    unsigned char *pTest = "HELLO SHA WORLD!";
    unsigned char abOuBuf[SHA256_DIGEST_LENGTH * 2];

    memset(abOuBuf, 0x00, sizeof(abOuBuf));
    Hash_SHA(pTest, strlen(pTest), abOuBuf);

    fprintf(stdout, "\n - Input : %s\n", pTest);
    fprintf(stdout, "\n - SHA256: %s\n", abOuBuf);
    return 0;
}