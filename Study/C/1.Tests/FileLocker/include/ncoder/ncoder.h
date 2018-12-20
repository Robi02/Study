#ifndef __NCODER_H__
#define __NCODER_H__

#include "common_define.h"

typedef enum _NCODE_TYPE
{
    NCODE_TYPE_HEXA     = 0,
    NCODE_TYPE_BASE64   = 1,
    // Add new NCODE_TYPE here...
    NCODE_TYPE_MAX

} NCODE_TYPE;

int Ncode(NCODE_TYPE eNcodeType, void *pstArgs, int szArgs);

#endif