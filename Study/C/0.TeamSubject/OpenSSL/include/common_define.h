#ifndef __COMMON_DEFINE_H__
#define __COMMON_DEFINE_H__

// [OS]
#ifdef __LINUX__
    #define _LINUX_OS_
#else
    #define _WINDOWS_OS_
#endif

// [System Bits]
#ifdef __X64__
    #define _X64_SYSTEM_
#else
    #define _X86_SYSTEM_
#endif

// [CPU Endian]
#ifdef __BIG_ENDIAN__
    #define _LITTLE_ENDIAN_CPU_
#else
    #define _BIG_ENDIAN_CPU_
#endif

// [rblib Common Typedef]
typedef unsigned char       BYTE;
typedef unsigned short      BYTE2;
typedef unsigned int        BYTE4;
typedef unsigned long long  BYTE8;

typedef          long long  INT64;
typedef unsigned long long  UINT64;

#endif