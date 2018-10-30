#include "common_include.h"
#include "rbtime.h"

static struct timeval gST_TIMEVAL;

void ResetTimer()
{
    memset(&gST_TIMEVAL, 0x00, sizeof(gST_TIMEVAL));
}

int StartTimer()
{
    return mingw_gettimeofday(&gST_TIMEVAL, NULL);
}

int StopTimer()
{
    struct timeval stStopTv;
    INT64  i64Time = 0;
    int    sec, ms, us;

    mingw_gettimeofday(&stStopTv, NULL);

    stStopTv.tv_sec  -= gST_TIMEVAL.tv_sec;
    stStopTv.tv_usec -= gST_TIMEVAL.tv_usec;
    i64Time = stStopTv.tv_sec * 1000000 + stStopTv.tv_usec;

    sec = i64Time / 1000000;        // Second
    ms  = i64Time % 1000000 / 1000; // Milli second (1/1000)
    us  = i64Time % 1000;           // Micro second (1/1000000)

    memcpy(&gST_TIMEVAL, &stStopTv, sizeof(gST_TIMEVAL));
    
    fprintf(stdout, "Time Elapsed : %d.%03d%03ds\n", sec, ms, us);

    return i64Time;
}

int GetTimeToString(TIME_TYPE eType, BYTE *pbBuf, int szBuf)
{
    time_t              sysTime;
    struct tm           *stLocTime;
    static struct tm    stLocTimeSaved;
    static BYTE         abBuf[TIME_MAX][32];
    int                 nYear, nMon, nDay, nHour, nMin, nSec, isCached;

    if (time(&sysTime) < 0) { return -1; }

    memset(pbBuf, 0x00, szBuf);

    if (eType == TIME_SYSTEM32)
    {
        if (szBuf < 11) { return -1; }
        return sprintf(pbBuf, "%010d", sysTime);
    }
    else if (eType == TIME_SYSTEM64)
    {
        if (szBuf < 21) { return -1; }
        return sprintf(pbBuf, "%020d", sysTime);
    }
    else
    {
        stLocTime = (struct tm *)localtime(&sysTime);
        nYear = stLocTime->tm_year + 1900;
        nMon  = stLocTime->tm_mon + 1;
        nDay  = stLocTime->tm_mday;
        nHour = stLocTime->tm_hour;
        nMin  = stLocTime->tm_min;
        nSec  = stLocTime->tm_sec;
        
        isCached = !memcmp(stLocTime, &stLocTimeSaved, sizeof(struct tm)); // Check 1sec caching
                                                                           // - Caching test : 1,000,000 no-delay case test
        if (isCached == 0)                                                 //   (caching: 1.8sec vs non-caching: 7.6sec)
        {                                                                  // - Result : sprintf has a little CPU overhead.
            memcpy(&stLocTimeSaved, stLocTime, sizeof(struct tm));
        }

        if (eType == TIME_YYYYMMDD)
        {
            const int nType = TIME_YYYYMMDD;
            const int szReq = 11; // "YYYY-MM-DD\0" (11byte)

            if (szBuf < szReq) { return -1; }
            
            if (isCached == 0)
            {
                sprintf(abBuf[nType], "%04d-%02d-%02d", nYear, nMon, nDay);
            }

            memcpy(pbBuf, abBuf[nType], szReq);
            return szReq - 1;
        }
        else if (eType == TIME_HHmmSS)
        {
            const int nType = TIME_HHmmSS;
            const int szReq = 9; // "HH:mm:SS\0" (9byte)

            if (szBuf < szReq) { return -1; }
            
            if (isCached == 0)
            {
                sprintf(abBuf[nType], "%02d:%02d:%02d", nHour, nMin, nSec);
            }

            memcpy(pbBuf, abBuf[nType], szReq);
            return szReq - 1;
        }
        else if (eType == TIME_YYYYMMDDHHmmSS)
        {
            const int nType = TIME_YYYYMMDDHHmmSS;
            const int szReq = 20; // "YYYY-MM-DD HH:mm:SS\0" (20byte)

            if (szBuf < szReq) { return -1; }
            
            if (isCached == 0)
            {
                sprintf(abBuf[nType], "%04d-%02d-%02d %02d:%02d:%02d", nYear, nMon, nDay, nHour, nMin, nSec);
            }

            memcpy(pbBuf, abBuf[nType], szReq);
            return szReq - 1;
        }
    }

    return -1;
}