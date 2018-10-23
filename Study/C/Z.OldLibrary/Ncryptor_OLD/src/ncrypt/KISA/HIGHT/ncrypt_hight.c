#include "ncryptor_define.h"
#include "ncrypt_common.h"
#include "KISA_HIGHT.h"
#include "ncrypt_hight.h"

static void Roundkey_HIGHT(BYTE *pbKey, int szKey, BYTE *pbRoundKey, int szRoundKey)
{
    HIGHT_KeySched(pbKey, (DWORD)szKey, pbRoundKey);
}

static void Encrypt_HIGHT(BYTE *pbBlock, int szBlock, BYTE *pbRoundKey, int szRoundKey)
{
    HIGHT_Encrypt(pbRoundKey, pbBlock);
}

static void Decrypt_HIGHT(BYTE *pbBlock, int szBlock, BYTE *pbRoundKey, int szRoundKey)
{
    HIGHT_Decrypt(pbRoundKey, pbBlock);
}

static int CheckArgs(NcryptArgs *pstArgs)
{
    return 0;
}

#define SZ_BLOCK 8
#define SZ_KEY 16
#define SZ_ROUND_KEY (SZ_KEY * 8 + 8)

int Ncrypt_HIGHT(NcryptArgs *pstArgs)
{
    /* KISA-HIGHT 64bit(8byte) fixed-size block en/decrypt algorithm */
    /* - Use 16byte key, 136byte round key, input 8byte stream block, output 8byte block(NULL padding) */
    static int  IS_KEY_INSERTED = 0;
    static BYTE PB_LAST_KEY[SZ_KEY];
    static BYTE PB_LAST_ROUND_KEY[SZ_ROUND_KEY];
    int rtVal;

    if ( (rtVal = CheckArgs(pstArgs)) != 0 )
    {
        LOG("CheckArgs() error. (return:%d)\n", rtVal);
        return rtVal;
    }

    if ( (rtVal = UpdateKeys(pstArgs, PB_LAST_KEY, SZ_KEY, PB_LAST_ROUND_KEY, SZ_ROUND_KEY, Roundkey_HIGHT, &IS_KEY_INSERTED)) != 0 )
    {
        LOG("UpdateKeys() error. (return:%d)\n", rtVal);
        return rtVal;
    }

    if ( (rtVal = EnDecryption(pstArgs, SZ_BLOCK, PB_LAST_ROUND_KEY, SZ_ROUND_KEY, Encrypt_HIGHT, Decrypt_HIGHT)) < 0)
    {
        LOG("EnDecryption() error. (return:%d)\n", rtVal);
        return rtVal;
    }

    return rtVal;
}