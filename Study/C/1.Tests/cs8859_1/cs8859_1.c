#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    FILE *fp = NULL;
    int i = 0;

    fp = fopen("test.txt", "w+");

    if (fp == NULL)
    {
        fprintf(stdout, "fopen() failed.\n");
        return -1;
    }

    for (i = 0; i <= 0xff; ++i)
    {
        unsigned char buf = (unsigned char)i;

        //if (iscntrl(buf))
        //{
        //    buf = '_';
        //}

        fwrite(&buf, 1, 1, fp);
        fprintf(stdout, "[%03d] : '%c'\n", i, buf);
    }

    fclose(fp);

    return 0;
}