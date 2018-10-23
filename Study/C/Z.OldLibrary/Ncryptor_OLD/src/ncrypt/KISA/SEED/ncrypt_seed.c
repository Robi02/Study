#include "ncryptor_define.h"
#include "ncrypt_common.h"
#include "KISA_SEED.h"
#include "ncrypt_seed.h"

static void Roundkey_SEED(BYTE *pbKey, int szKey, BYTE *pbRoundKey, int szRoundKey)
{
    SEED_KeySchedKey((DWORD*)pbRoundKey, pbKey);
}

static void Encrypt_SEED(BYTE *pbBlock, int szBlock, BYTE *pbRoundKey, int szRoundKey)
{
    SEED_Encrypt(pbBlock, (DWORD*)pbRoundKey);
}

static void Decrypt_SEED(BYTE *pbBlock, int szBlock, BYTE *pbRoundKey, int szRoundKey)
{
    SEED_Decrypt(pbBlock, (DWORD*)pbRoundKey);
}

static int CheckArgs(NcryptArgs *pstArgs)
{
    return 0;
}

#define SZ_BLOCK 16
#define SZ_KEY 16
#define SZ_ROUND_KEY (SZ_KEY * 8)

int Ncrypt_SEED(NcryptArgs *pstArgs)
{
    /* KISA-SEED 128bit(16byte) fixed-size block en/decrypt algorithm */
    /* - Use 16byte key, 128byte round key, input 16byte stream block, output 16byte stream block(NULL padding) */
    static int  IS_KEY_INSERTED = 0;
    static BYTE PB_LAST_KEY[SZ_KEY];
    static BYTE PB_LAST_ROUND_KEY[SZ_ROUND_KEY];
    int rtVal;

    if ( (rtVal = CheckArgs(pstArgs)) != 0 )
    {
        LOG("CheckArgs() error. (return:%d)\n", rtVal);
        return rtVal;
    }

    if ( (rtVal = UpdateKeys(pstArgs, PB_LAST_KEY, SZ_KEY, PB_LAST_ROUND_KEY, SZ_ROUND_KEY, Roundkey_SEED, &IS_KEY_INSERTED)) != 0 )
    {
        LOG("UpdateKeys() error. (return:%d)\n", rtVal);
        return rtVal;
    }

    if ( (rtVal = EnDecryption(pstArgs, SZ_BLOCK, PB_LAST_ROUND_KEY, SZ_ROUND_KEY, Encrypt_SEED, Decrypt_SEED)) < 0)
    {
        LOG("EnDecryption() error. (return:%d)\n", rtVal);
        return rtVal;
    }

    return rtVal;
}