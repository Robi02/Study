#include "KISA/ARIA/ncrypt_aria.h"
#include "KISA/HIGHT/ncrypt_hight.h"
#include "KISA/SEED/ncrypt_seed.h"
#include "ncryptor_define.h"
#include "ncrypt.h"

static int CheckArgs(NcryptArgs *pstArgs)
{
    const NCRYPT_WORKTYPE   eWorktype  = pstArgs->eWorktype;
    const NCRYPT_ALGORITHM  eAlgorithm = pstArgs->eAlgorithm;
    const NCRYPT_MODE       eMode      = pstArgs->eMode;
    const NCRYPT_PADDING    ePadding   = pstArgs->ePadding;
    const unsigned char     *pInStream = pstArgs->pInStream;
    const int               szInStream = pstArgs->szInStream;
    const unsigned char     *pOutBuf   = pstArgs->pOutBuf;
    const int               szOutBuf   = pstArgs->szOutBuf;
    const unsigned char     *pKey      = pstArgs->pKey;

    if ( eWorktype < 0 || eWorktype >= NW_MAX )
    {
        LOG("Undefined eWorktype value. (eWorktype:%d)\n", eWorktype);
        return -1;
    }

    if ( eAlgorithm < 0 || eAlgorithm >= NA_MAX )
    {
        LOG("Undefined eAlgorithm value. (eAlgorithm:%d)\n", eAlgorithm);
        return -1;
    }

    if ( eMode < 0 || eMode >= NM_MAX )
    {
        LOG("Undefined eMode value. (eMode:%d)\n", eMode);
        return -1;
    }

    if ( ePadding < 0 || ePadding >= NP_MAX )
    {
        LOG("Undefined ePadding value. (ePadding:%d)\n", ePadding);
        return -1;
    }

    if ( pInStream == NULL || szInStream <= 0 )
    {
        LOG("InStream error. (pInStream:0x%p, szInStream:%d)\n", pInStream, szInStream);
        return -1;
    }

    if ( pOutBuf == NULL || szOutBuf <= 0 )
    {
        LOG("OutBuf error. (pOutBuf:0x%p, szOutBuf:%d)\n", pOutBuf, szOutBuf);
        return -1; 
    }

    if ( pKey == NULL )
    {
        LOG("pKey error. (pKey:0x%p)\n", pKey);
        return -1; 
    }

    return 0;
}

static int NcryptWork(NcryptArgs *pstArgs)
{
    NCRYPT_WORKTYPE     eWorktype  = pstArgs->eWorktype;
    NCRYPT_ALGORITHM    eAlgorithm = pstArgs->eAlgorithm;

    if ( eAlgorithm == NA_SEED )
    {
        return Ncrypt_SEED(pstArgs);
    }
    else if ( eAlgorithm == NA_HIGHT )
    {
        return Ncrypt_HIGHT(pstArgs);
    }
    else if ( eAlgorithm == NA_ARIA )
    {
        return Ncrypt_ARIA(pstArgs);
    }
    else if ( 0 )
    {
        /* Add new en/decryption algorithm here... */
    }
    else
    {   
        LOG("Undefined eWorktype or eAlgorithm. (eWorktype:%d, eAlgorithm:%d)\n", eWorktype, eAlgorithm);
        return -1;
    }

    return -1;
}

int Ncrypt(NCRYPT_WORKTYPE eWorktype, NCRYPT_ALGORITHM eAlgorithm, NCRYPT_MODE eMode, NCRYPT_PADDING ePadding,
           unsigned char *pInStream, int szInStream, unsigned char *pOutBuf, int szOutBuf,
           unsigned char *pKey, unsigned char *pIV)
{
    NcryptArgs stArgs = { eWorktype, eAlgorithm, eMode, ePadding,
                          pInStream, szInStream, pOutBuf, szOutBuf, pKey, pIV };
    int rtVal;
    
    if ( (rtVal = CheckArgs(&stArgs)) != 0 )
    {
        LOG("CheckArgs() returns error. (return:%d)\n", rtVal);
        return -1;
    }

    if ( (rtVal = NcryptWork(&stArgs)) < 0 )
    {
        LOG("NcryptWork() returns error. (return:%d)\n", rtVal);
        return -1;
    }

    return rtVal;
}