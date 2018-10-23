#include "rbtime.h"

static struct timeval gST_TIMEVAL;

void ResetTimer()
{
    memset(gST_TIMEVAL, 0x00, sizeof(gST_TIMEVAL));
}

int StartTimer()
{
    return gettimeofday(&gST_TIMEVAL, NULL);
}

int StopTimer()
{
    struct timeval stStopTv;

    gettimeofday(&stStopTv, NULL);

    stStopTv.tv_sec  -= gST_TIMEVAL.tv_sec;
    stStopTv.tv_usec -= gST_TIMEVAL.tv_usec;
    
    fprintf(stdout, "Time Elapsed : %d.%ds\n", stStopTv.tv_sec, stStopTv.tv_usec);

    return stStopTv.tv_sec + stStopTv.tv_usec;
}