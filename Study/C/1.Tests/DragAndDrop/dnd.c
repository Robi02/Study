#include <stdio.h>
#include <windows.h>

int main(int argc, char **argv)
{
    if (argc == 2)
        fprintf(stdout, "����� �巡���� ���α׷����� '%s' �Դϴ�.\n", argv[1]);
    else
        fprintf(stdout, "���α׷��� �巡���� �ּ���.\n");

    Sleep(5000);

    return 0;
}