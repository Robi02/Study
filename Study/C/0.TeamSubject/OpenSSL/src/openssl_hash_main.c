#include "openssl_hash_main.h"

int Hash_SHA(unsigned char *pInStr, size_t szInStr, unsigned char *pOuBuf)
{
    SHA256_CTX ctx;

    /* Text binary to SHA256 */
    if (SHA256_Init(&ctx) == 0)                    { fprintf(stdout, "SHA256_Init() Error!\n"); return -1; }
    if (SHA256_Update(&ctx, pInStr, szInStr) == 0) { fprintf(stdout, "SHA256_Update() Error!\n"); return -1; }
    if (SHA256_Final(pOuBuf, &ctx) == 0)           { fprintf(stdout, "SHA256_Final() Error!\n"); return -1; }

    return 0;
}

int Hash_md5(unsigned char *pInStr, size_t szInStr, unsigned char *pOuBuf)
{
    if (MD5(pInStr, szInStr, pOuBuf) == NULL)
    {
        fprintf(stdout, "MD5() Error!\n");
        return -1;
    }
}

void PrintUsage()
{
    fprintf(stdout, "[hashcrypt Usage]\n");
    fprintf(stdout, "hashcrypt.exe [Type] [String]\n");
    fprintf(stdout, " [Type]\n");
    fprintf(stdout, "  1. -sha256 : SHA-256 (Output:32byte)\n");
    fprintf(stdout, "  2. -md5    : MD5 (Output:16byte)\n");
}

int main(int argc, char **argv)
{
    unsigned char   abBuf[2048];
    unsigned char   abOuBuf[SHA256_DIGEST_LENGTH];
    int             strLen = 0;

    if (argc != 3) { PrintUsage(); return -1; }

    memset(abBuf, 0x00, sizeof(abBuf));
    memset(abOuBuf, 0x00, sizeof(abOuBuf));

    strLen = strlen(argv[2]);
    memcpy(abBuf, argv[2], strLen);
    
    fprintf(stdout, "\n - Input : %s\n", abBuf);

    if (strcmp(argv[1], "-sha256") == 0)
    {
        Hash_SHA(abBuf, min(strLen, sizeof(abBuf)), abOuBuf);
        fprintf(stdout, "\n - SHA256(HEX): ");
        PrintStreamToHexa(abOuBuf, sizeof(abOuBuf));
    }
    else
    {
        Hash_md5(abBuf, min(strLen, sizeof(abBuf)), abOuBuf);
        fprintf(stdout, "\n - MD5(HEX): ");
        PrintStreamToHexa(abOuBuf, sizeof(abOuBuf) / 2);
    }

    return 0;
}