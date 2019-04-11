#include <stdio.h>
#include "lib1.h"
#include "lib2.h"

int main(int argc, char **argv)
{
    fprintf(stdout, "Hello CMake World!\n");
    lib1_main();
    lib2_main();
    return 0;
}