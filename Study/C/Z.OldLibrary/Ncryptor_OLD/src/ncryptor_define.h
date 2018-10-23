#ifndef __NCRYPTOR_DEFINE_H__
#define __NCRYPTOR_DEFINE_H__

#include <stdio.h>

#define DEBUG 1

#define LOG(format, ...) \
do { if (DEBUG) { fprintf(stderr, "[%s(%d)::%s() Error!] " format, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); } } while (0);

#endif