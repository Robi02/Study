#ifndef __TIME_UTIL_H__
#define __TIME_UTIL_H__

#include "common.h"

namespace robi_util {

class TimeUtil {
private:
    TimeUtil();
    ~TimeUtil();
public:
    static int64    GetCurrentTimeMillis();
    static int      GetSimpleDateFormat(char *out_buf, int out_buf_len, const char *strftime_fmt, int64 time_millis);

};}

#endif