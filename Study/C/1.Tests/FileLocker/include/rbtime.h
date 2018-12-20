#ifndef __RBTIME_H__
#define __RBTIME_H__

#include "common_define.h"

typedef enum _TIME_TYPE
{
    TIME_SYSTEM32       = 0,    // The number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC), x86 length. (11byte)
    TIME_SYSTEM64       = 1,    // x64 length. (21byte)
    TIME_YYYYMMDD       = 2,    // Year-Month-Date (11byte)
    TIME_HHmmSS         = 3,    // Hour:Min:Sec (9byte)
    TIME_YYYYMMDDHHmmSS = 4,    // Year-Month-Date Hour:Min:Sec (20byte)
    TIME_MAX

} TIME_TYPE;

void ResetTimer();
int StartTimer();
int StopTimer();
int GetTimeToString(TIME_TYPE eType, BYTE *pbBuf, int szBuf);

#endif