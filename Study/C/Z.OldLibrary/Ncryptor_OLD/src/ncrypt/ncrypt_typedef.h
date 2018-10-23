#ifndef __NCRYPT_TYPEDEF_H__
#define __NCRYPT_TYPEDEF_H__

typedef enum NCRYPT_WORKTYPE
{
    NW_ENCRYPT = 0,
    NW_DECRYPT = 1,
    NW_MAX

} NCRYPT_WORKTYPE;

typedef enum _NCRYPT_ALGORITHM
{
    NA_SEED  = 0,
    NA_HIGHT = 1,
    NA_ARIA  = 2,
    NA_MAX

} NCRYPT_ALGORITHM;

typedef enum _NCRYPT_MODE
{
    NM_ECB = 0,
    NM_CBC = 1,
    NM_MAX

} NCRYPT_MODE;

typedef enum _NCRYPT_PADDING
{
    NP_NULL = 0,
    NP_PKCS = 1,
    NP_MAX

} NCRYPT_PADDING;

typedef struct _NcryptArgs
{
    NCRYPT_WORKTYPE     eWorktype;
    NCRYPT_ALGORITHM    eAlgorithm;
    NCRYPT_MODE         eMode;
    NCRYPT_PADDING      ePadding;
    unsigned char       *pInStream;
    int                 szInStream;
    unsigned char       *pOutBuf;
    int                 szOutBuf;
    unsigned char       *pKey;
    unsigned char       *pIV;

} NcryptArgs;

#endif