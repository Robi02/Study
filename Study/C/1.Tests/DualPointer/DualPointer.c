#include <stdio.h>
#include <stdlib.h>

typedef struct _Warp
{
    int             value;
    struct _Warp    *pNextWarp;

} Warp;

void PrintWarp(Warp *pWarp)
{
    if (pWarp != NULL)
    {
        fprintf(stdout, "This      : 0x%p\n", pWarp);
        fprintf(stdout, "Value     : %d\n", pWarp->value);
        fprintf(stdout, "pNextWarp : 0x%p\n", pWarp->pNextWarp);
        fprintf(stdout, "----------------------------------------------\n");
    }
    else
    {
        fprintf(stdout, "pWarp is NULL!\n");
        fprintf(stdout, "----------------------------------------------\n");
    }
}

void GenWarp(Warp *pGenWarp, int value, Warp *pNextWarp)
{
    pGenWarp = (Warp*)malloc(sizeof(Warp)); // pGenWarp포인터 변수는 main() 문의 pstWarp의 주소를 보관하고 있다가, 
    pGenWarp->value     = value;            // malloc()이 호출되는 순간 새로 할당된 주소를 가르키게 된다.
    pGenWarp->pNextWarp = pNextWarp;        // 그리고 새로 할당된 주소의 데이터를 갱신한다.

    fprintf(stdout, "[Inside of GenWarp() Function]\n");
    PrintWarp(pGenWarp);                    // 여기서 출력하면 당연히 새 주소의 데이터를 잘 출력한다.
}

void DestroyWarp(Warp *pSrcWarp)
{
    if (pSrcWarp != NULL)
    {
        pSrcWarp->pNextWarp = NULL;
        free(pSrcWarp);
    }
}

void SetWarp(Warp *pWarp, int value, Warp *pNextWarp)
{
    pWarp->value     = value;
    pWarp->pNextWarp = pNextWarp;
}

int main(int argc, char **argv)
{
    Warp *pstWarp1 = NULL, *pstWarp2 = NULL, *pstWarp3 = NULL;

    GenWarp(pstWarp1, 10, NULL);
    PrintWarp(pstWarp1);            // 이 시점에서 pstWarp1의 값은? 그대로 NULL이다. 포인터가 가진 '주소값을 복사'
                                    // 하여 PirntWarp()로 넘겨준거지 '참조'를 넘겨준게 아니기 때문.
    GenWarp(pstWarp2, 20, NULL);
    PrintWarp(pstWarp2);

    GenWarp(pstWarp3, 30, NULL);
    PrintWarp(pstWarp3);

    DestroyWarp(pstWarp1);
    DestroyWarp(pstWarp2);
    DestroyWarp(pstWarp3);

    return 0;
}