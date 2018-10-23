#include "common_include.h"
#include "ncode_hexa.h"

/* [ HEXA Encoding ]
 *
 * 1. Synopsis
 *
 *  < Binary 8bit >            < Char >   < Decimal >   < Hexa >
 *  [0][0][1][1][1][0][1][0] =    :     =     58      =    3A
 *
 *  > 1byte = 8bit / 8bit = 2hex base / 1byte = 2hex base
 *  > BaseSymbol : 0123456789ABCDEF
 *
 * 2. Example
 *
 *  < Input Str > "KOREA"
 *
 *  < Binary > 01001011(K) 01001111(O) 01010010(R) 01000101(E) 01000001(A)
 *
 *  < Hexa > 4(0100)B(1011) 4(0100)F(1111) 5(0101)2(0010) 4(0100)5(0101) 4(0100)1(0001)
 *
 *  < Result > "4B4F524541"
 *
 */

static int CheckCommonArgs(NCODE_TYPE_HEXA_ARGS *pstArgs);
static int CheckEncodeArgs(NCODE_TYPE_HEXA_ARGS *pstArgs);
static int CheckDecodeArgs(NCODE_TYPE_HEXA_ARGS *pstArgs);

static const char *P_HEXA_SYMBOL = "0123456789ABCDEFabcdef";
static const BYTE A_HEXA_REVERSE_TABLE[256] = 
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9, 255, 255, 255, 255, 255, 255, 
    255,  10,  11,  12,  13,  14,  15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255,  10,  11,  12,  13,  14,  15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

static int CheckCommonArgs(NCODE_TYPE_HEXA_ARGS *pstArgs)
{
    if (pstArgs == NULL)
    {
        fprintf(stderr, "pstArgs is NULL.\n");
        return -1;
    }

    if (pstArgs->pbInStream == NULL || pstArgs->szInStream < 1)
    {
        fprintf(stderr, "pbInStream is NULL || szInStream < 1 (pbInStream:0x%p, szInStream:%d)\n", pstArgs->pbInStream, pstArgs->szInStream);
        return -1;
    }

    if (pstArgs->pbOutBuf == NULL || pstArgs->szOutBuf < 1)
    {
        fprintf(stderr, "pbOutBuf is NULL || szOutBuf < 1 (pbOutBuf:0x%p, szOutBuf:%d)\n", pstArgs->pbOutBuf, pstArgs->szOutBuf);
        return -1;
    }

    return 0;
}

static int CheckEncodeArgs(NCODE_TYPE_HEXA_ARGS *pstArgs)
{
    int szOutReq = pstArgs->szInStream * 2;

    if (pstArgs->szOutBuf < szOutReq)
    {
        fprintf(stderr, "szOutBuf < szOutReq (%d < %d)\n", pstArgs->szOutBuf, szOutReq);
        return -1;
    }

    return 0;
}

static int CheckDecodeArgs(NCODE_TYPE_HEXA_ARGS *pstArgs)
{
    int szOutReq = pstArgs->szInStream / 2;

    if (pstArgs->szOutBuf < szOutReq)
    {
        fprintf(stderr, "szOutBuf < szOutReq (%d < %d)\n", pstArgs->szOutBuf, szOutReq);
        return -1;
    }

    if (pstArgs->szInStream < 2)
    {
        fprintf(stderr, "szInStream < 2 (%d < 2)\n", pstArgs->szInStream);
        return -1;
    }

    return 0;
}

static BYTE Combine4Bit(NCODE_TYPE_HEXA_ARGS *pstArgs, BYTE bLeft4Bit, BYTE bRight4Bit)
{
    BYTE bTemp, bByte;
 
    bTemp = bByte = 0;

    if ((bTemp = A_HEXA_REVERSE_TABLE[bLeft4Bit]) == 255)
    {
        fprintf(stderr, "Inadaptable character '%c' for HEXA symbol.\n", bLeft4Bit);
        return (BYTE)-1;
    }

    bLeft4Bit = bTemp;
    
    if ((bTemp = A_HEXA_REVERSE_TABLE[bRight4Bit]) == 255)
    {
        fprintf(stderr, "Inadaptable character '%c' for HEXA symbol.\n", bRight4Bit);
        return (BYTE)-1;
    }

    bRight4Bit = bTemp;

    bByte = bLeft4Bit << 4;
    bByte |= bRight4Bit;

    return bByte;
}

int Encode_Hexa(NCODE_TYPE_HEXA_ARGS *pstArgs)
{
    int     inIdx, outIdx, rtVal;
    BYTE    bByte, bLeft4Bit, bRight4Bit;

    inIdx = outIdx = rtVal = 0;
    bByte = bLeft4Bit = bRight4Bit = 0;

    if ((rtVal = CheckCommonArgs(pstArgs)) < 0)
    {
        fprintf(stderr, "CheckCommonArgs() error. (return:%d)\n", rtVal);
        return rtVal;
    }

    if ((rtVal = CheckEncodeArgs(pstArgs)) < 0)
    {
        fprintf(stderr, "CheckEncodeArgs() error. (return:%d)\n", rtVal);
        return rtVal;
    }

    for (inIdx = 0; inIdx < pstArgs->szInStream; ++inIdx)
    {
        bByte      = pstArgs->pbInStream[inIdx];
        bLeft4Bit  = bByte >> 4;
        bRight4Bit = bByte &= 0x0F;

        pstArgs->pbOutBuf[outIdx++] = P_HEXA_SYMBOL[bLeft4Bit  + pstArgs->isLowercase * 6];
        pstArgs->pbOutBuf[outIdx++] = P_HEXA_SYMBOL[bRight4Bit + pstArgs->isLowercase * 6];
    }

    return outIdx;
}

int Decode_Hexa(NCODE_TYPE_HEXA_ARGS *pstArgs)
{
    int     inIdx, outIdx, rtVal;
    BYTE    bByte, bLeft4Bit, bRight4Bit;

    inIdx = outIdx = rtVal = 0;
    bByte = bLeft4Bit = bRight4Bit = 0;

    if ((rtVal = CheckCommonArgs(pstArgs)) < 0)
    {
        fprintf(stderr, "CheckCommonArgs() error. (return:%d)\n", rtVal);
        return rtVal;
    }

    if ((rtVal = CheckDecodeArgs(pstArgs)) < 0)
    {
        fprintf(stderr, "CheckDecodeArgs() error. (return:%d)\n", rtVal);
        return rtVal;
    }

    for (inIdx = 0; inIdx < pstArgs->szInStream; )
    {
        bLeft4Bit  = pstArgs->pbInStream[inIdx++];
        bRight4Bit = pstArgs->pbInStream[inIdx++];
        
        if ((bByte = Combine4Bit(pstArgs, bLeft4Bit, bRight4Bit)) == (BYTE)-1)
        {
            fprintf(stderr, "Combine4Bit() error. (return:(BYTE)%d)\n", bByte);
            return -1;
        }
        
        pstArgs->pbOutBuf[outIdx++] = bByte;
    }

    return outIdx;
}