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
    pGenWarp = (Warp*)malloc(sizeof(Warp)); // pGenWarp������ ������ main() ���� pstWarp�� �ּҸ� �����ϰ� �ִٰ�, 
    pGenWarp->value     = value;            // malloc()�� ȣ��Ǵ� ���� ���� �Ҵ�� �ּҸ� ����Ű�� �ȴ�.
    pGenWarp->pNextWarp = pNextWarp;        // �׸��� ���� �Ҵ�� �ּ��� �����͸� �����Ѵ�.

    fprintf(stdout, "[Inside of GenWarp() Function]\n");
    PrintWarp(pGenWarp);                    // ���⼭ ����ϸ� �翬�� �� �ּ��� �����͸� �� ����Ѵ�.
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
    PrintWarp(pstWarp1);            // �� �������� pstWarp1�� ����? �״�� NULL�̴�. �����Ͱ� ���� '�ּҰ��� ����'
                                    // �Ͽ� PirntWarp()�� �Ѱ��ذ��� '����'�� �Ѱ��ذ� �ƴϱ� ����.
    GenWarp(pstWarp2, 20, NULL);
    PrintWarp(pstWarp2);

    GenWarp(pstWarp3, 30, NULL);
    PrintWarp(pstWarp3);

    DestroyWarp(pstWarp1);
    DestroyWarp(pstWarp2);
    DestroyWarp(pstWarp3);

    return 0;
}