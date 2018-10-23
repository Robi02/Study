#include <stdio.h>
#include <stdint.h>

typedef uint32_t BYTE4;
typedef uint16_t BYTE2;
typedef uint8_t  BYTE;

int main(int argc, char **argv)
{
    BYTE4 TARGET4 = 0x10203040;
    BYTE  TARGET1 = 0x12;
    BYTE  *pByte = NULL;

    fprintf(stdout, "\n[TARGET4]\n");
    for (int i = 0; i < sizeof(BYTE4); ++i)
    {
        pByte = ((BYTE *)&TARGET4 + i);
        fprintf(stdout, "Byte[%d]:%0X\n", i, *pByte);
    }

    fprintf(stdout, "\n[TARGET1]\n");
    for (int i = 0; i < sizeof(BYTE); ++i)
    {
        pByte = ((BYTE *)&TARGET1 + i);
        fprintf(stdout, "Byte[%d]:%0X\n", i, *pByte);
    }

    return 0;
}