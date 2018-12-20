#include <stdio.h>
#include <string.h>

#ifndef PROTOTYPES
    #define PROTOTYPES 0
#endif

#if PROTOTYPES
    #define PROTO_LIST(list) list
#else
    #define PROTO_LIST(list) ()
#endif

int prototype_func PROTO_LIST((pInStr, szIn, pOutStr, szOut));
int main PROTO_LIST((argc, argv));

int prototype_func(pInStr, szIn, pOutStr, szOut)
char *pInStr;
size_t szIn;
char *pOutStr;
size_t szOut;
{
    int szCpy = (szIn < szOut ? szIn : szOut);

    strncpy(pOutStr, pInStr, szCpy + 1);

    return szCpy;
}

int main(argc, argv)
int argc;
char **argv;
{
    char    *pTestStrA = "Hello World!";
    char    arTestBufA[255];
    int     rtVal;

    rtVal = prototype_func(pTestStrA, strlen(pTestStrA), arTestBufA, sizeof(arTestBufA));

    fprintf(stdout, "pTestStrA : %s\narTestBufA : %s", pTestStrA, arTestBufA);

    return rtVal;
}