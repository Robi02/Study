#include "util/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    Logger::Info("Hello Test!\n");
    Logger::Info("Hello test Log! (%s)\n", "test_varg");
    Logger::Info("Hello test Log! (%d)\n", 12345);
    return 0;
}