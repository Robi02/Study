#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "rbmath.h"
#include "ncryptor_define.h"
#include "ncrypt_common.h"

static int CheckCommonArgs(NcryptArgs *pstArgs, int szBlock)
{
    NCRYPT_MODE     eWorktype   = pstArgs->eWorktype;
    NCRYPT_PADDING  ePadding    = pstArgs->ePadding;
    int             szInStream  = pstArgs->szInStream;
    int             szOutBuf    = pstArgs->szOutBuf;

    if ( ePadding == NP_PKCS )
    {
        if ( eWorktype == NW_ENCRYPT && (szInStream % szBlock == 0 && szOutBuf < szInStream + szBlock) )
        {
            /* PKCS encryption padding need 1 more block when szInsteram is multiple of blocksize(szBlock) */
            LOG("(szOutBuf < szInStream + %d). (ePadding:%d, szOutBuf:%d, szInStream:%d)\n", 
                szBlock, ePadding, szOutBuf, szInStream);
            return -1;
        }
    }
    else if ( szInStream > szOutBuf )
    {
        /* Ignore size when padding mode is NP_PKCS */
        LOG("(szInStream > szOutBuf). (szInStream:%d, szOutBuf:%d)\n", szInStream, szOutBuf);
        return -1;
    }

    return 0;
}

/* Cut block-size data from input steram with padding option */
static int GetBlockFromStream(BYTE *pbOutBlock, int szOutBlock, BYTE *pbInStream, int szRemain, NCRYPT_PADDING ePadding)
{
    if ( szRemain < 0 )
    {
        LOG("(szRemain < 0). (szRemain:%d)\n", szRemain);
        return -1;
    }

    memset(pbOutBlock, 0x00, szOutBlock);
    memcpy(pbOutBlock, pbInStream, min(szRemain, szOutBlock));

    if ( szRemain <= szOutBlock ) /* When remain block size is over szOutBlock, last block may need padding */
    {
        int szPadding = szOutBlock - szRemain;

        if ( ePadding == NP_NULL )
        {
            /* NP_NULL: Fill up empty array with NULL */
            memset(&pbOutBlock[szOutBlock - szPadding], 0x00, szPadding);
        }
        else if ( ePadding == NP_PKCS )
        {
            if ( szPadding == szOutBlock )
            {
                /* NP_PKCS: When padding size(szRemain) is 'szOutBlock', fillup pbOutBlock with block size */
                memset(pbOutBlock, szOutBlock, szOutBlock);
            }
            else
            {
                /* NP_PKCS: Fill up emtpy pbOutBlock with szRemain */
                memset(&pbOutBlock[szOutBlock - szPadding], szPadding, szPadding);
            }
        }
    }

    return 0;
}

static int XorStream(BYTE *pbStream_L, int szStream_L, const BYTE *pbStream_R, const int szStream_R)
{
    int i;

    if ( szStream_L != szStream_R )
    {
        LOG("szStream_L != pbStream_R (szStream_L:%d, pbStream_R:%d)\n", szStream_L, szStream_R);
        return -1;
    }

    for ( i = 0; i < szStream_L; ++i )
    {
        pbStream_L[i] ^= pbStream_R[i];
    }

    return 0;
}

int UpdateKeys(NcryptArgs *pstArgs, BYTE *pbKey, int szKey, BYTE *pbRoundKey, int szRoundKey,
               void (*pKeyFunc)(BYTE*, int, BYTE*, int), int *pIsKeyInserted)
{
    if ( pKeyFunc == NULL )
    {
        LOG("pKeyFunc is NULL. (pKeyFunc:0x%p)\n", pKeyFunc);
        return -1;
    }

    if ( (*pIsKeyInserted == 1) && (memcmp(pstArgs->pKey, pbKey, szKey) == 0) ) 
    {
        return 0; /* Key not changed, use last round key */
    }
    
    /* Key changed, get new round key */
    memcpy(pbKey, pstArgs->pKey, szKey);
    pKeyFunc(pbKey, szKey, pbRoundKey, szRoundKey);
    *pIsKeyInserted = 1;

    return 0;
}

int EnDecryption(NcryptArgs *pstArgs, int szBlock, BYTE *pbRoundKey, int szRoundKey,
                 void (*pEncFunc)(BYTE*, int, BYTE*, int), void (*pDecFunc)(BYTE*, int, BYTE*, int))
{
    NCRYPT_WORKTYPE eWorktype   = pstArgs->eWorktype;
    NCRYPT_MODE     eMode       = pstArgs->eMode;
    NCRYPT_PADDING  ePadding    = pstArgs->ePadding;
    BYTE            *pbInStream = (BYTE*)pstArgs->pInStream;
    BYTE            *pbOutBuf   = (BYTE*)pstArgs->pOutBuf;
    BYTE            pbBlock[szBlock];
    BYTE            pbIV[szBlock];
    int             szInStream  = pstArgs->szInStream;
    int             szRemain    = szInStream;
    int             nBlock      = ((szInStream + szBlock - 1) / szBlock);
    int             idx, rtVal, i;

    if ( pEncFunc == NULL || pDecFunc == NULL )
    {
        LOG("pEncFunc or pDecFunc is NULL. (pEncFunc:0x%p, pDecFunc:0x%p)\n", pEncFunc, pDecFunc);
        return -1;
    }

    if ( (rtVal = CheckCommonArgs(pstArgs, szBlock)) != 0 )
    {
        LOG("CheckCommonArgs() error. (return:%d)\n", rtVal);
        return rtVal;
    }

    if ( eWorktype == NW_ENCRYPT && ePadding == NP_PKCS && szInStream % szBlock == 0 )
    {
        /* When last block padding size is '0', PKCS makes one more block and fillup that with block size */
        ++nBlock;
    }

    if ( eMode == NM_CBC )
    {
        if ( pstArgs->pIV == NULL )
        {
            LOG("NcryptMode is CBC but, pIV is NULL. (pstArgs->pIV:0x%p)\n", pstArgs->pIV);
            return -1;
        }

        memcpy(pbIV, pstArgs->pIV, szBlock);
    }

    for ( i = 0, idx = 0; i < nBlock; ++i )
    {
        if ( (rtVal = GetBlockFromStream(pbBlock, szBlock, &pbInStream[idx], szRemain, ePadding)) != 0 )
        {
            LOG("GetBlock() error. (return:%d)\n", rtVal);
            return rtVal;
        }

        if ( eWorktype == NW_ENCRYPT ) /* Encryption */
        {
            if ( eMode == NM_CBC ) /* Enc-CBC Mode */
            {
                XorStream(pbBlock, szBlock, pbIV, szBlock);
                pEncFunc(pbBlock, szBlock, pbRoundKey, szRoundKey);
                memcpy(pbIV, pbBlock, szBlock);
            }
            else /* Enc-ECB Mode */
            {
                pEncFunc(pbBlock, szBlock, pbRoundKey, szRoundKey);
            }
        }
        else /* Decryption */
        {
            if ( eMode == NM_CBC ) /* Dec-CBC Mode */
            {
                BYTE pbCipherStream[szBlock];

                memcpy(pbCipherStream, pbBlock, szBlock);
                pDecFunc(pbBlock, szBlock, pbRoundKey, szRoundKey);
                XorStream(pbBlock, szBlock, pbIV, szBlock);
                memcpy(pbIV, pbCipherStream, szBlock);
            }
            else /* Dec-ECB Mode */
            {
                pDecFunc(pbBlock, szBlock, pbRoundKey, szRoundKey);
            }
        }

        memcpy(&pbOutBuf[idx], pbBlock, szBlock);
        szRemain -= szBlock;
        idx += szBlock;
    }

    return idx;
}

void PrintStreamToHexa(const unsigned char *pStream, int szStream)
{
    char pBuf[szStream * 3 + 1];
    int i;

    memset(pBuf, 0x00, sizeof(pBuf));

    for ( i = 0; i < szStream; ++i )
    {
        sprintf(&pBuf[i * 3], "%02X ", pStream[i]);
    }

    fprintf(stdout, "%s\n", pBuf);
}

void PrintStreamToString(const unsigned char *pStream, int szStream)
{
    char pBuf[szStream + 1];
    int i;

    memset(pBuf, 0x00, sizeof(pBuf));

    for ( i = 0; i < szStream; ++i )
    {
        if ( iscntrl(pStream[i]) != 0 )
        {
            pBuf[i] = '_';
        }
        else
        {
            pBuf[i] = pStream[i];
        }
    }

    fprintf(stdout, "[%s]\n", pBuf);
}

void GenerateStream(unsigned char *pStream, int szStream, int optRandom)
{
    int i;

    for ( i = 0; i < szStream; ++i )
    {
        if (optRandom == 0)
        {
            pStream[i] = (unsigned char)0x00;
        }
        else if (optRandom == -1)
        {
            pStream[i] = (unsigned char)(rand() % 256);
        }
        else if (optRandom == -2)
        {
            pStream[i] = (unsigned char)i;
        }
        else
        {
            pStream[i] = (unsigned char)optRandom;
        }
    }
}

void GenerateStreamFromString(unsigned char *pStream, int szStream, unsigned char *pStr)
{
    int szStr = strlen(pStr);
    int i;

    memcpy(pStream, pStr, min(szStream, szStr));
}