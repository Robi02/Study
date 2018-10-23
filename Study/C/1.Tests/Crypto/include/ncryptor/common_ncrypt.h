#ifndef __COMMOM_NCRYPT_H__
#define __COMMOM_NCRYPT_H__

#include "common_typedef.h"

// Algorithms //
typedef enum _NCRYPT_ALGO
{
    NCRYPT_ALGO_SEED        = 0, // SEED
    NCRYPT_ALGO_HIGHT       = 1, // HIGHT
    NCRYPT_ALGO_ARIA        = 2, // ARIA12, ARIA14, ARIA16
    NCRYPT_ALGO_DES         = 3, // DES, DES3
    NCRYPT_ALGO_AES         = 4, // AES
    NCRYPT_ALGO_BLOWFISH    = 5, // BLOWFISH
    // Add new NCRYPT_ALGO here...
    NCRYPT_ALGO_MAX

} NCRYPT_ALGO;

// Block Mode //
typedef enum _NCRYPT_MODE
{
    NCRYPT_MODE_ECB = 0, // ECB(Electric Code Book)
    NCRYPT_MODE_CBC = 1, // CBC(Chiper Block Chaining)
    // Add new NCRYPT_MODE here...
    NCRYPT_MODE_MAX

} NCRYPT_MODE;

// Padding //
typedef enum _NCRYPT_PADD
{
    NCRYPT_PADD_NULL = 0, // NULL
    NCRYPT_PADD_PKCS = 1, // PKCS5/7
    // Add new NCRYPT_PADD here...
    NCRYPT_PADD_MAX

} NCRYPT_PADD;

// Block Cipher(ARIA, AES, Blowfish, DES, HIGHT, SEED, ...) Common Args //
typedef struct _NCRYPT_COMMON_BLOCK_ARGS
{
    int         isEncrypt;      // '1' is encryption, others are decryption.
    NCRYPT_ALGO eAlgo;          // Algorithm that want to use.
    NCRYPT_PADD ePadd;          // Padding option.
    NCRYPT_MODE eMode;          // Mode option.
    BYTE        *pbIV;          // Initial Vector(IV) for CBC mode.
    int         szIV;           // Byte size of IV.
    BYTE        *pbKey;         // En/decryption key.
    int         szKey;          // Byte size of key.
    BYTE        *pbInStream;    // Input stream to en/decrypting.
    int         szInStream;     // Byte size of input stream.
    BYTE        *pbOutBuf;      // Output stream buffer.
    int         szOutBuf;       // Byte size of output buffer.

} NCRYPT_COMMON_BLOCK_ARGS;

int Ncryption(NCRYPT_COMMON_BLOCK_ARGS *pstArgs, int szBlock, BYTE *pbRoundKey, int szRoundKey,
              void (*pAlgoFunc)(BYTE*, int, BYTE*, int, void*));

#endif