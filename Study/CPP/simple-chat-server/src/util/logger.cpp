#include "util/logger.h"
#include "util/time_util.h"
#include <cstring>
#include <ctime>
#include <stdarg.h>
#include <stdlib.h>

const char *Logger::kDebugStr   = "[DBG]";
const char *Logger::kTraceStr   = "[TRC]";
const char *Logger::kInfoStr    = "[INF]";
const char *Logger::kWarningStr = "[WAN]";
const char *Logger::kErrorStr   = "[ERR]";
const char *Logger::kFatalStr   = "[FAT]";

void Logger::Debug(const char *log_str, ...)
{ va_list varg_list; va_start(varg_list, log_str); Logger::Log(Level::DEBUG,   log_str, &varg_list); va_end(varg_list); }
void Logger::Trace(const char *log_str, ...)
{ va_list varg_list; va_start(varg_list, log_str); Logger::Log(Level::TRACE,   log_str, &varg_list); va_end(varg_list); }
void Logger::Info(const char *log_str, ...)
{ va_list varg_list; va_start(varg_list, log_str); Logger::Log(Level::INFO ,   log_str, &varg_list); va_end(varg_list); }
void Logger::Warning(const char *log_str, ...)
{ va_list varg_list; va_start(varg_list, log_str); Logger::Log(Level::WARNING, log_str, &varg_list); va_end(varg_list); }
void Logger::Error(const char *log_str, ...)
{ va_list varg_list; va_start(varg_list, log_str); Logger::Log(Level::ERROR,   log_str, &varg_list); va_end(varg_list); }
void Logger::Fatal(const char *log_str, ...)
{ va_list varg_list; va_start(varg_list, log_str); Logger::Log(Level::FATAL,   log_str, &varg_list); va_end(varg_list); }

void Logger::Log(Level log_level, const char *log_str, va_list *varg_list)
{
    // Time str
    time_t raw_time;
    struct tm *time_info;
    char time_str_buf[32];  // "[yyyy-MM-dd HH:mm:ss]" (22bytes)

    time(&raw_time);
    time_info = localtime(&raw_time);
    strftime(time_str_buf, sizeof(time_str_buf), "[%F %T]", time_info);

    // Level str
    const char *level_str = nullptr;

    switch (log_level)
    {
        case Level::DEBUG:
            level_str = kDebugStr;
            break;
        case Level::TRACE:
            level_str = kTraceStr;
            break;
        case Level::INFO:
            level_str = kInfoStr;
            break;
        case Level::WARNING:
            level_str = kWarningStr;
            break;
        case Level::ERROR:
            level_str = kErrorStr;
            break;
        case Level::FATAL:
        default:
            level_str = kFatalStr;
            break;
    }

    // Output stream
    FILE *out_fd = nullptr;

    if (log_level == Level::ERROR || log_level == Level::FATAL)
    {
        out_fd = stderr;
    }
    else
    {
        out_fd = stdout;
    }

    // Log formatting
    if (log_str == nullptr)
    {
        log_str = "null";
    }

    int log_fmt_len = strlen(time_str_buf) +strlen(level_str) + strlen(log_str) + 7; // +7 => ' ' : 2 / "---" : 4 / '\0' : 1
    char log_fmt[log_fmt_len];

    sprintf(log_fmt, "%s %s --- %s\n", time_str_buf, level_str, log_str);
    
    // Print log
    if (log_str != nullptr)
    {
        vfprintf(out_fd, log_fmt, *varg_list);
    }
    else
    {
        fprintf(out_fd, log_fmt);
    }
}