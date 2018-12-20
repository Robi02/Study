#ifndef __NCODE_BASE64_H__
#define __NCODE_BASE64_H__

#include "common_define.h"

typedef struct _NCODE_TYPE_BASE64_ARGS
{
    int     isEncode;       // 1 is encoding, the other is decoding
    BYTE    *pbInStream;    // Input stream to en/decoding
    int     szInStream;     // Byte size of input stream
    BYTE    *pbOutBuf;      // Output stream buffer
    int     szOutBuf;       // Byte size of output buffer
    
} NCODE_TYPE_BASE64_ARGS;

int Encode_Base64(NCODE_TYPE_BASE64_ARGS *pstArgs);
int Decode_Base64(NCODE_TYPE_BASE64_ARGS *pstArgs);

#endif