#ifndef __RBLOG_H__
#define __RBLOG_H__

#include "common_define.h"

typedef enum _RBLOG_LEVEL
{
    LOG_FATAL = 1, // Logging to 'error log' file, Exit program.
    LOG_ERROR = 2, // Logging to 'error log' file.
    LOG_WARN  = 3, // Logging to 'warn log' file.
    LOG_INFO  = 4, // Logging to 'common log' file.
    LOG_DEBUG = 5, // Logging to stdout.
    LOG_MAX

} RBLOG_LEVEL;

#define LOG_FATAL(fmt, ...) do { Log(LOG_FATAL, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); } while (0);
#define LOG_ERROR(fmt, ...) do { Log(LOG_ERROR, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); } while (0);
#define LOG_WARN(fmt, ...)  do { Log(LOG_WARN,  __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); } while (0);
#define LOG_INFO(fmt, ...)  do { Log(LOG_INFO,  __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); } while (0);
#define LOG_DEBUG(fmt, ...) do { Log(LOG_DEBUG, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); } while (0);

void Log(const RBLOG_LEVEL eLv, const BYTE *pbSrc, const int nLine, const BYTE *pbFunc, const BYTE *pbFmt, ...);

#endif