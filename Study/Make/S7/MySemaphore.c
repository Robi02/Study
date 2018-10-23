#include <stdio.h>
#include "Common_Include.h"
#include "MySemaphore.h"

#ifdef __DEBUG__
    #define LOG_DEBUG(fmt, ...) do { fprintf(stderr, "[%s(%d)::%s()] " fmt, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); } while (0);
#else
    #define LOG_DEBUG
#endif

int MySemaphore()
{
    LOG_DEBUG("MySemaphore()\n", NULL);
    return 0;
}