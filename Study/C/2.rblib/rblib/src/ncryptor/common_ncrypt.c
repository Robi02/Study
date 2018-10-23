#include <stdio.h>
#include <string.h>
#include "rbmath.h"
#include "common_ncrypt.h"

static int CheckCommonArgs(NCRYPT_COMMON_BLOCK_ARGS *pstArgs, int szBlock)
{
    NCRYPT_ALGO eAlgo       = pstArgs->eAlgo;
    NCRYPT_MODE eMode       = pstArgs->eMode;
    NCRYPT_PADD	ePadd		= pstArgs->ePadd;
    int         szInStream  = pstArgs->szInStream;
    int         szOutBuf    = pstArgs->szOutBuf;

    if (ePadd == NCRYPT_PADD_PKCS)
    {
        if ((pstArgs->isEncrypt == 1) && (szInStream % szBlock == 0 && szOutBuf < szInStream + szBlock))
        {
            /* PKCS encryption padding need 1 more block when szInsteram is multiple of blocksize(szBlock) */
            fprintf(stderr, "(szOutBuf < szInStream + %d). (ePadd:%d, szOutBuf:%d, szInStream:%d)\n", 
					szBlock, ePadd, szOutBuf, szInStream);
            return -1;
        }
    }
    else if (szInStream > szOutBuf)
    {
        /* Ignore size when padding mode is NP_PKCS */
        fprintf(stderr, "(szInStream > szOutBuf). (szInStream:%d, szOutBuf:%d)\n", szInStream, szOutBuf);
        return -1;
    }

    if (pstArgs->pbInStream == NULL || szInStream < 1)
    {
        fprintf(stderr, "pbInStream == NULL || szInStream < 1 (pbInSteram:0x%p, szInStream:%d)\n", pstArgs->pbInStream, szInStream);
        return -1;
    }

    if (pstArgs->pbOutBuf == NULL || szOutBuf < 1)
    {
        fprintf(stderr, "pbOutBuf == NULL || szOutBuf < 1 (pbOutBuf:0x%p, szOutBuf:%d)\n", pstArgs->pbOutBuf, szOutBuf);
        return -1;
    }

    if (eAlgo < 0 || eAlgo >= NCRYPT_ALGO_MAX)
    {
        fprintf(stderr, "Undefined eAlgo value. (eAlgo:%d)\n", eAlgo);
        return -1;
    }

    if (ePadd < 0 || ePadd >= NCRYPT_PADD_MAX)
    {
        fprintf(stderr, "Undefined ePadd value. (ePadd:%d)\n", ePadd);
        return -1;
    }

    if (eMode < 0 || eMode >= NCRYPT_MODE_MAX)
    {
        fprintf(stderr, "Undefined eMode value. (eMode:%d)\n", eMode);
        return -1; 
    }

    if (eMode == NCRYPT_MODE_CBC && (pstArgs->pbIV == NULL || pstArgs->szIV < szBlock))
    {
        fprintf(stderr, "NcryptMode is CBC but, pbIV is NULL || szIV < szBlock. \
                (pstArgs->pbIV:0x%p, szIV:%d < szBlock:%d)\n", pstArgs->pbIV, pstArgs->szIV, szBlock);
        return -1;
    }

    return 0;
}

static int XorStream(BYTE *pbStream_L, int szStream_L, const BYTE *pbStream_R, const int szStream_R)
{
    int i;

    if (szStream_L != szStream_R)
    {
        fprintf(stderr, "szStream_L != pbStream_R (szStream_L:%d, pbStream_R:%d)\n", szStream_L, szStream_R);
        return -1;
    }

    for (i = 0; i < szStream_L; ++i)
    {
        pbStream_L[i] ^= pbStream_R[i];
    }

    return 0;
}

/* Cut block-size data from input steram with padding option */
static int GetBlockFromStream(BYTE *pbOutBlock, int szOutBlock, BYTE *pbInStream, int szRemain, NCRYPT_PADD ePadd)
{
    if (szRemain < 0)
    {
        fprintf(stderr, "(szRemain < 0). (szRemain:%d)\n", szRemain);
        return -1;
    }

    memset(pbOutBlock, 0x00, szOutBlock);
    memcpy(pbOutBlock, pbInStream, min(szRemain, szOutBlock));

    if (szRemain <= szOutBlock) /* When remain block size is over szOutBlock, last block may need padding */
    {
        int szPadding = szOutBlock - szRemain;

        if (ePadd == NCRYPT_PADD_NULL)
        {
            /* NP_NULL: Fill up empty array with NULL */
            memset(&pbOutBlock[szOutBlock - szPadding], 0x00, szPadding);
        }
        else if (ePadd == NCRYPT_PADD_PKCS)
        {
            if (szPadding == szOutBlock)
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

int Ncryption(NCRYPT_COMMON_BLOCK_ARGS *pstArgs, int szBlock, BYTE *pbRoundKey, int szRoundKey,
              void (*pAlgoFunc)(BYTE*, int, BYTE*, int, void*))
{
	int			isEncrypt   = pstArgs->isEncrypt;
	NCRYPT_MODE eMode		= pstArgs->eMode;
    NCRYPT_PADD ePadd		= pstArgs->ePadd;
    BYTE        *pbInStream = pstArgs->pbInStream;
    BYTE        *pbOutBuf   = pstArgs->pbOutBuf;
    BYTE        pbBlock[szBlock];
    BYTE        pbIV[szBlock];
    int         szInStream  = pstArgs->szInStream;
    int         szRemain    = szInStream;
    int         nBlock      = ((szInStream + szBlock - 1) / szBlock);
    int         outIdx, rtVal, i;

    if ((rtVal = CheckCommonArgs(pstArgs, szBlock)) != 0)
    {
        fprintf(stderr, "CheckCommonArgs() error. (return:%d)\n", rtVal);
        return -1;
    }

    if (pAlgoFunc == NULL)
    {
        fprintf(stderr, "pAlgoFunc == NULL. (pEncFunc:0x%p)\n", pAlgoFunc);
        return -1;
    }

    if (pbRoundKey == NULL || szRoundKey < 1)
    {
        fprintf(stderr, "pbRoundKey == NULL || szRoundKey < 1 (pbRoundKey:0x%p, szRoundKey:%d)\n", pbRoundKey, szRoundKey);
        return -1;
    }

    if (eMode == NCRYPT_MODE_CBC) // CBC Mode
    {
        memcpy(pbIV, pstArgs->pbIV, szBlock);
    }

    if ((isEncrypt == 1) && (ePadd == NCRYPT_PADD_PKCS && szInStream % szBlock == 0)) // PKCS Padd
    {
        /* When last block padding size is '0', PKCS makes one more block and fillup that with block size */
        ++nBlock;
    }

    for (i = 0, outIdx = 0; i < nBlock; ++i)
    {
        if ((rtVal = GetBlockFromStream(pbBlock, szBlock, &pbInStream[outIdx], szRemain, ePadd)) != 0)
        {
            fprintf(stderr, "GetBlockFromStream() error. (return:%d)\n", rtVal);
            return -1;
        }

        if (isEncrypt == 1) /* Encryption */
        {
            if (eMode == NCRYPT_MODE_CBC) /* Enc-CBC Mode */
            {
                XorStream(pbBlock, szBlock, pbIV, szBlock);
                pAlgoFunc(pbBlock, szBlock, pbRoundKey, szRoundKey, pstArgs);
                memcpy(pbIV, pbBlock, szBlock);
            }
            else /* Enc-ECB Mode */
            {
                pAlgoFunc(pbBlock, szBlock, pbRoundKey, szRoundKey, pstArgs);
            }
        }
        else /* Decryption */
        {
            if (eMode == NCRYPT_MODE_CBC) /* Dec-CBC Mode */
            {
                BYTE pbCipherStream[szBlock];

                memcpy(pbCipherStream, pbBlock, szBlock);
                pAlgoFunc(pbBlock, szBlock, pbRoundKey, szRoundKey, pstArgs);
                XorStream(pbBlock, szBlock, pbIV, szBlock);
                memcpy(pbIV, pbCipherStream, szBlock);
            }
            else /* Dec-ECB Mode */
            {
                pAlgoFunc(pbBlock, szBlock, pbRoundKey, szRoundKey, pstArgs);
            }
        }

        memcpy(&pbOutBuf[outIdx], pbBlock, szBlock);
        szRemain -= szBlock;
        outIdx += szBlock;
    }

    return outIdx;
}