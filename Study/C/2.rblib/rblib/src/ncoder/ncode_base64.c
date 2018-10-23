#include "common_include.h"
#include "ncode_base64.h"

/* [ BASE64 Encoding ]
 *
 * 1. Synopsis
 *
 * < 3Char >           < Binary 24bit >                  < Base64 6bit binary >		   < Base64 >
 *   "Man"      [01001101][01100001][01101110]		[010011][010110][000101][101110]     "TWFu"
 *
 *  > Make group each 6bits for 4 base64 char and padding '=' for empty space.
 *  > BaseSymbol: ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/
 *
 * 2. Example
 *
 *  < Input Str > "KOREA"
 *
 *  < Binary > 01001011(K) 01001111(O) 01010010(R) 01000101(E) 01000001(A)
 *
 *  < Base64 6bit binary > [010010(S)110100(0)111101(9)010010(S)][010001(R)010100(U)0001/00(E)(=)]
 *
 *  < Result > "S09SRUE="
 *
 */

static int CheckCommonArgs(NCODE_TYPE_BASE64_ARGS *pstArgs);
static int CheckEncodeArgs(NCODE_TYPE_BASE64_ARGS *pstArgs);
static int CheckDecodeArgs(NCODE_TYPE_BASE64_ARGS *pstArgs);

static const char A_BASE64_PADDING[1] = { '=' };
static const char *P_BASE64_SYMBOL = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const BYTE A_BASE64_REVERSE_TABLE[256] =
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 255, 255, 255, 
    255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255, 
    255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

static int CheckCommonArgs(NCODE_TYPE_BASE64_ARGS *pstArgs)
{
    if (pstArgs == NULL)
    {
        fprintf(stderr, "pstArgs is NULL\n");
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

static int CheckEncodeArgs(NCODE_TYPE_BASE64_ARGS *pstArgs)
{
    // [ szInStream -> szOutReq ]
    // [1 -> 4(xx==)], [4 -> 8], [7 -> 12], ... 
    // [2 -> 4(xxx=)], [5 -> 8], [8 -> 12],
    // [3 -> 4(xxxx)], [6 -> 8], [9 -> 12],

    int szOutReq = ((pstArgs->szInStream + 2) / 3) * 4;

    if (pstArgs->szOutBuf < szOutReq)
    {
        fprintf(stderr, "szOutBuf < szOutReq (%d < %d)\n", pstArgs->szOutBuf, szOutReq);
        return -1;
    }

    return 0;
}

static int CheckDecodeArgs(NCODE_TYPE_BASE64_ARGS *pstArgs)
{
    int szInStream  = pstArgs->szInStream;

    if (szInStream % 4 != 0)
    {
        fprintf(stderr, "szInStream %% 4 != 0 (szInStream:%d)\n", szInStream);
        return -1;
    }
    
    // [ szInStream -> szOutReq ]
    // [4(xx==) -> 1], [8(xxxxyy==) -> 4], ...
    // [4(xxx=) -> 2], [8(xxxxyyy=) -> 5],
    // [4(xxxx) -> 3], [8(xxxxyyyy) -> 6],

    BYTE *pbInStream = pstArgs->pbInStream;
    int  paddCnt = CountStreamInStream(&pbInStream[szInStream - 4], 4, A_BASE64_PADDING, 1);

    if (paddCnt > 2)
    {
        fprintf(stderr, "Count of padding symbol '%c' must be under 2. (paddCnt:%d > 2)\n", A_BASE64_PADDING[0], paddCnt);
        return -1;
    }

    int szOutReq = ((pstArgs->szInStream * 3) / 4) - paddCnt;

    if (pstArgs->szOutBuf < szOutReq)
    {
        fprintf(stderr, "szOutBuf < szOutReq (%d < %d)\n", pstArgs->szOutBuf, szOutReq);
        return -1;
    }

    return 0;
}


int Encode_Base64(NCODE_TYPE_BASE64_ARGS *pstArgs)
{
    BYTE    abByteBuf[3], ab6BitBuf[4];
    int     szInStream = pstArgs->szInStream, szOutBuf = pstArgs->szOutBuf;
    int     inIdx, outIdx, szRemain, szBlock, rtVal;

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

    for (inIdx = outIdx = 0; inIdx < szInStream; inIdx += szBlock)
    {
        memset(abByteBuf, 0x00, 3);
        memset(ab6BitBuf, 0x00, 4);

        szRemain = szInStream - inIdx;
        szBlock  = min(szRemain, 3);

        memcpy(abByteBuf, &pstArgs->pbInStream[inIdx], szBlock);

        ab6BitBuf[0] =  (abByteBuf[0] & 0xFC) >> 2;                                 // [wwwwww00 00000000 00000000]
        ab6BitBuf[1] = ((abByteBuf[0] & 0x03) << 4) | ((abByteBuf[1] & 0xF0) >> 4); // [000000xx xxxx0000 00000000]
        ab6BitBuf[2] = ((abByteBuf[1] & 0x0F) << 2) | ((abByteBuf[2] & 0xC0) >> 6); // [00000000 0000yyyy yy000000]
        ab6BitBuf[3] =  (abByteBuf[2] & 0x3F);                                      // [00000000 00000000 00zzzzzz]

        for (int i = 0 ; i < 4; ++i)
        {
            pstArgs->pbOutBuf[outIdx++] = P_BASE64_SYMBOL[ab6BitBuf[i]];
        }
    }

    if (szBlock > 0)
    {
        memset(&pstArgs->pbOutBuf[outIdx + szBlock - 3], *A_BASE64_PADDING, 3 - szBlock); // Last block padding
    }

    return outIdx;
}

int Decode_Base64(NCODE_TYPE_BASE64_ARGS *pstArgs)
{
    BYTE    ab6BitBuf[4], bByte;
    BYTE4   b4Buf;
    int     szInStream = pstArgs->szInStream, szOutBuf = pstArgs->szOutBuf;
    int     inIdx, outIdx, szPadd, rtVal;

    szPadd = 0;

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

    for (inIdx = outIdx = 0; inIdx < szInStream; inIdx += 4)
    {
        memset(&b4Buf, 0x00000000, 4);
        memcpy(ab6BitBuf, &pstArgs->pbInStream[inIdx], 4);

        for (int i = 0; i < 4; ++i)
        {
            b4Buf <<= 6;

            if ((bByte = A_BASE64_REVERSE_TABLE[ab6BitBuf[i]]) < 64)
            {
                if (szPadd > 0) // Prevent input like [S09SRU=E]
                {
                    fprintf(stderr, "Input stream couldn't be placed after padding symbol '%c'. (Idx:%d)\n", A_BASE64_PADDING[0], inIdx + i);
                    return -1;
                }

                b4Buf |= (BYTE4)bByte; // Expand BYTE to BYTE4 and append to b4Buf.
            }
            else if (ab6BitBuf[i] == A_BASE64_PADDING[0])
            {
                if (inIdx + i < szInStream - 2) // Prevent input like [S09SR===]
                {
                    fprintf(stderr, "Padding symbol '%c' couldn't be placed in the middle of stream. (Idx:%d)\n", A_BASE64_PADDING[0], inIdx + i);
                    return -1;
                }
                
                ++szPadd;
            }
            else
            {
                fprintf(stderr, "Inadaptable character '%c' for BASE64 symbol.\n", ab6BitBuf[i]);
                return -1;
            }
        }

        int szBlock = 3 - szPadd; // Last block's szPadd value will 0~2, other blocks are always 0.

        for (int i = 0; i < szBlock; ++i) // So, szBlock's value is always between 1~3.
        {
            // b4Buf : [00000000 wwwwwwxx xxxxyyyy yyzzzzzz]
            pstArgs->pbOutBuf[outIdx++] = (b4Buf >> ((2 - i) * 8)) & 0x000000FF;
        }
    }

    return outIdx;
}