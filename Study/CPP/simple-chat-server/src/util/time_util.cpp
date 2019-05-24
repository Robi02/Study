#include "util/time_util.h"
#include <chrono>
#include <sys/time.h>

namespace robi_util {

int64 TimeUtil::GetCurrentTimeMillis() {
    // [Note] <sys/time.h> is much FASTER than C++ <chrono>
    //  1) <chrono>     1,000,000case -> 4091ms
    //  2) <sys/time.h> 1,000,000case -> 3794ms (almost 7.25% FASTER)
    //  
    // [C++ code]
    // namespace sc = std::chrono;
    // return (int64) sc::duration_cast<sc::milliseconds>(sc::system_clock::now().time_since_epoch()).count();
    
    // [C code]
    struct timeval time_val;
    gettimeofday(&time_val, nullptr);
    return (int64)(time_val.tv_sec * 1000 + time_val.tv_usec / 1000);
}

int TimeUtil::GetSimpleDateFormat(char *out_buf, int out_buf_len, const char *strftime_fmt, int64 time_millis)
{
    if (out_buf == nullptr || out_buf_len < 1)
    {
        Logger::Error("'out_buf' is null or 'out_buf_len' < 1 error! (out_buf_len:%d)", out_buf_len);
        return -1;
    }

    if (strftime_fmt == nullptr)
    {
        Logger::Error("'strftime_fmt' is null!");
        return -1;
    }

    if (time_millis < 0)
    {
        
        Logger::Error("'time_millis' < 0 error!");
        return -1;
    }

    // [Note] strtime format reference
    // - http://www.cplusplus.com/reference/ctime/strftime
    
    time_t raw_time_sec = (time_millis / 1000L);
    struct tm *time_info;

    time_info = localtime(&raw_time_sec);
    strftime(out_buf, out_buf_len, strftime_fmt, time_info);
    return strlen(out_buf);
}}