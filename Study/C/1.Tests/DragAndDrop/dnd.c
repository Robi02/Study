#include <stdio.h>
#include <windows.h>

int main(int argc, char **argv)
{
    if (argc == 2)
        fprintf(stdout, "당신이 드래그한 프로그램명은 '%s' 입니다.\n", argv[1]);
    else
        fprintf(stdout, "프로그램을 드래그해 주세요.\n");

    Sleep(5000);

    return 0;
}