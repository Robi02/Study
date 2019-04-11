#include <stdio.h>
#include "lib2.h"

int lib2_main()
{
    fprintf(stdout, "lib2_main()!\n");
    lib2_logic1();
    return 0;
}

void lib2_logic1()
{
    fprintf(stdout, "lib2_logic1()!\n");
}