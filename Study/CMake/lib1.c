#include <stdio.h>
#include "lib1.h"

int lib1_main()
{
    fprintf(stdout, "lib1_main()!\n");
    lib1_logic1();
    return 0;
}

void lib1_logic1()
{
    fprintf(stdout, "lib1_logic1()!\n");
}