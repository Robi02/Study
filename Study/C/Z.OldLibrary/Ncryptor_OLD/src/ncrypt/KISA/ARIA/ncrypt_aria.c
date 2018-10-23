#include "ncryptor_define.h"
#include "ncrypt_common.h"
#include "KISA_ARIA.h"
#include "ncrypt_ARIA.h"

static void Roundkey_ARIA_ENC(BYTE *pbKey, int szKey, BYTE *pbRoundKey, int szRoundKey)
{
    ARIA_KeySched(pbKey, (DWORD)szKey, pbRoundKey, 0);
}

static void Roundkey_ARIA_DEC(BYTE *pbKey, int szKey, BYTE *pbRoundKey, int szRoundKey)
{
    ARIA_KeySched(pbKey, (DWORD)szKey, pbRoundKey, 1);
}

static void Encrypt_ARIA(BYTE *pbBlock, int szBlock, BYTE *pbRoundKey, int szRoundKey)
{
    ARIA_Encrypt(pbRoundKey, pbBlock, szBlock);
}

static void Decrypt_ARIA(BYTE *pbBlock, int szBlock, BYTE *pbRoundKey, int szRoundKey)
{
    ARIA_Decrypt(pbRoundKey, pbBlock, szBlock);
}

static int CheckArgs(NcryptArgs *pstArgs)
{
    return 0;
}

#define SZ_BLOCK 16
#define SZ_KEY 16                           /* 16Byte (12round), 24Byte (14round), 32Byte (16round) */
#define SZ_ROUND_KEY (SZ_KEY * (12 + 1))    /* 208byte((12+1)*16), 240byte, 272byte */

int Ncrypt_ARIA(NcryptArgs *pstArgs)
{
    /* KISA-ARIA 128bit(16byte) fixed-size block en/decrypt algorithm */
    /* - Use 16/24/32byte key(each 12/14/16 rounds), 208//byte round key,
    /*   input 16byte stream block, output 16byte block(NULL padding) */
    /* - But, this source support 16byte key(12round) only. */
    static int  IS_ENC_KEY_INSERTED = 0;
    static int  IS_DEC_KEY_INSERTED = 0;
    static BYTE PB_LAST_KEY[SZ_KEY];
    static BYTE PB_LAST_ENC_ROUND_KEY[SZ_ROUND_KEY];
    static BYTE PB_LAST_DEC_ROUND_KEY[SZ_ROUND_KEY];
    BYTE *pRoundKey = NULL;
    int rtVal;

    if ( (rtVal = CheckArgs(pstArgs)) != 0 )
    {
        LOG("CheckArgs() error. (return:%d)\n", rtVal);
        return rtVal;
    }

    /* Encryption round key */
    if ( (rtVal = UpdateKeys(pstArgs, PB_LAST_KEY, SZ_KEY, PB_LAST_ENC_ROUND_KEY, SZ_ROUND_KEY, Roundkey_ARIA_ENC, &IS_ENC_KEY_INSERTED)) != 0 )
    {
        LOG("UpdateKeys():Encryption error. (return:%d)\n", rtVal);
        return rtVal;
    }

    /* Decryption round key */
    if ( (rtVal = UpdateKeys(pstArgs, PB_LAST_KEY, SZ_KEY, PB_LAST_DEC_ROUND_KEY, SZ_ROUND_KEY, Roundkey_ARIA_DEC, &IS_DEC_KEY_INSERTED)) != 0 )
    {
        LOG("UpdateKeys():Decryption error. (return:%d)\n", rtVal);
        return rtVal;
    }

    pRoundKey = (pstArgs->eWorktype == NW_ENCRYPT ? PB_LAST_ENC_ROUND_KEY : PB_LAST_DEC_ROUND_KEY);

    if ( (rtVal = EnDecryption(pstArgs, SZ_BLOCK, pRoundKey, SZ_ROUND_KEY, Encrypt_ARIA, Decrypt_ARIA)) < 0 )
    {
        LOG("EnDecryption() error. (return:%d)\n", rtVal);
        return rtVal;
    }

    return rtVal;
}