#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <cstring>
#include <errno.h>
#include <stdio.h>

class Logger
{
public:
    enum Level
    {
        UNKNOWN = -1, DEBUG, TRACE, INFO, WARNING, ERROR, FATAL, MAX
    };

private:
    static const char *kDebugStr;
    static const char *kTraceStr;
    static const char *kInfoStr;
    static const char *kWarningStr;
    static const char *kErrorStr;
    static const char *kFatalStr;

private:
    Logger();
    ~Logger();
    static void Log(Level log_level, const char *log_str, va_list *varg_list);

public:
    static void Debug(const char *log_str, ...);
    static void Trace(const char *log_str, ...);
    static void Info(const char *log_str, ...);
    static void Warning(const char *log_str, ...);
    static void Error(const char *log_str, ...);
    static void Fatal(const char *log_str, ...);
};

#endif