#include "common_include.h"
#include "bytestream.h"

int CountStreamInStream(const BYTE *pbIn, int szIn, const BYTE *pbFind, int szFind)
{
    int iLimit, jFound, rtCnt;
    int i, j;

    if (pbIn == NULL || szIn < 1)
    {
        fprintf(stderr, "pbIn == NULL || szIn < 1 (pbIn:0x%p, szIn:%d)\n", pbIn, szIn);
        return -1;
    }

    if (pbFind == NULL || szFind < 1)
    {
        fprintf(stderr, "pbIn == NULL || szIn < 1 (pbIn:0x%p, szIn:%d)\n", pbIn, szIn);
        return -1;
    }

    if (szIn < szFind)
    {
        fprintf(stderr, "szIn < szFind (%d < %d)\n", szIn, szFind);
        return -1;
    }

    iLimit = szIn - szFind + 1;
    jFound = rtCnt = 0;

    for (i = 0; i < iLimit; ++i)
    {
        jFound = 1;

        for (j = 0; j < szFind; ++j)
        {
            if (pbIn[i + j] != pbFind[j])
            {
                jFound = 0;
                break;
            }
        }

        if (jFound)
        {
            i += szFind - 1;
            ++rtCnt;
        }
    }

    return rtCnt;
}

void PrintStreamToBinary(FILE *pFd, const BYTE *pbIn, int szIn)
{
    BYTE    abBuf[szIn * 8 + 1];
    BYTE    bByte, b1Bit;
    int     i, j, k;

    if (pbIn == NULL || szIn < 1)
    {
        fprintf(stderr, "pbIn == NULL || szIn < 1 (pbIn:0x%p, szIn:%d)\n", pbIn, szIn);
        return;
    }

    for (i = szIn - 1, k = 0; i > -1; --i)
    {
        bByte = pbIn[i];

        for (j = 0; j < 8; ++j)
        {
            sprintf(&abBuf[k++], "%d", (bByte >> j) & 0x01);
        }
    }

    fprintf(pFd, "%s", abBuf);
}

void PrintStreamToHexa(const BYTE *pbStream, int szStream)
{
    char pBuf[szStream * 3 + 1];
    int i;

    memset(pBuf, 0x00, sizeof(pBuf));

    for ( i = 0; i < szStream; ++i )
    {
        sprintf(&pBuf[i * 3], "%02X ", pbStream[i]);
    }

    fprintf(stdout, "%s\n", pBuf);
}

void PrintStreamToString(const BYTE *pbStream, int szStream)
{
    char pBuf[szStream + 1];
    int i;

    memset(pBuf, 0x00, sizeof(pBuf));

    for ( i = 0; i < szStream; ++i )
    {
        if ( iscntrl(pbStream[i]) != 0 )
        {
            pBuf[i] = '_';
        }
        else
        {
            pBuf[i] = pbStream[i];
        }
    }

    fprintf(stdout, "[%s]\n", pBuf);
}

void StreamFromString(BYTE *pbStream, int szStream, BYTE *pbStr)
{
    int szStr = strlen(pbStr);

    memcpy(pbStream, pbStr, min(szStream, szStr));
}