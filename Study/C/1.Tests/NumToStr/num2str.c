#include <stdio.h>
#include <string.h>

static const int    SZ_BUF = 1024;

static const char   *SYM_KOREAN[]   = { "°ø", "ÀÏ", "ÀÌ", "»ï", "»ç", "¿À", "À°", "Ä¥", "ÆÈ", "±¸" };
static const char   *SYM_NUMBER[]   = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static const int    CNT_SYMBOLS     = sizeof(SYM_NUMBER) / sizeof(char*);

static char* getTokenFromStr(char *pIn, char *pOut, char *pDelim)
{
    int i, dlmLen, strLen, chkLen;

    strLen = strlen(pIn);
    dlmLen = strlen(pDelim);
    chkLen = strLen - dlmLen + 1;

    for (i = 0; i < chkLen; ++i)
    {
        if (strcmp(pIn[i], pDelim) == 0)
        {
            memcpy(pOut, pIn, i);
            break;
        }
    }

    return pIn + i;
}

static int cvt_num2kor(char *pBuf, char *pRegex)
{
    int     i, lnLen, nCnt = 0;
    char    aBuf[SZ_BUF];

    for (i = 0; i < CNT_SYMBOLS; ++i) // For each number symbols in line
    {
        const char  *pSymNum = SYM_NUMBER[i];
        const int   symNumLen = strlen(pSymNum);
        const char  *pSymKor = SYM_KOREAN[i];
        const int   symKorLen = strlen(pSymKor);
        char        *pBufOffset = pBuf;

        // Change number symbols to korean symbols
        while ((pBufOffset = strstr(pBufOffset + symNumLen, pSymNum)) != NULL)
        {
            char    *pCatStr = pBufOffset + symNumLen;
            char    *pToken = NULL;
            int     tokenLen = 0;

            memcpy(pBufOffset + symKorLen, pCatStr, strlen(pCatStr));   // Push back origin string
            memcpy(pBufOffset, pSymKor, symKorLen);                     // Write korean symbol
            ++nCnt;
        }
    }

    return nCnt;
}

static int cvt_file_n_save(char *pFileName, char *pRegex)
{
    FILE    *infp = NULL, *oufp = NULL;
    char    aBuf[SZ_BUF];
    int     i, nCnt = 0;

    memset(aBuf, 0x00, sizeof(aBuf));
    memcpy(aBuf, pFileName, strlen(pFileName));
    strcat(aBuf, "_cvt");

    infp = fopen(pFileName, "r+");
    oufp = fopen(aBuf, "w+");

    if (infp == NULL) // pFileName open fail
    {
        fprintf(stdout, "Cannot find the file '%s'.\n", pFileName);
        return -1;
    }

    if (oufp == NULL) // pFileName_cvt create fail
    {
        fprintf(stdout, "Cannot create the file '%s'.\n", aBuf);
        return -1;
    }

    memset(aBuf, 0x00, sizeof(aBuf));

    while (fgets(aBuf, sizeof(aBuf), infp) != NULL) // Get line from file
    {
        int lnLen = 0;

        nCnt += cvt_num2kor(aBuf, pRegex);
        lnLen = strlen(aBuf);
        fwrite(aBuf, sizeof(char), lnLen, oufp);

        memset(aBuf, 0x00, sizeof(aBuf));
    }

    fclose(infp);
    fclose(oufp);

    return nCnt;
}

static int cvt_str_n_print(char *pStr, char *pRegex)
{
    char    aBuf[SZ_BUF];
    int     nCnt = 0;

    memset(aBuf, 0x00, sizeof(aBuf));
    memcpy(aBuf, pStr, strlen(pStr));

    nCnt = cvt_num2kor(aBuf, pRegex);

    fprintf(stdout, "RESULT : %s\n", aBuf);

    return nCnt;
}

int main(int argc, char **argv)
{
    int rtVal = 0;

    if (strcmp(argv[1], "-f") == 0)
    {
        if ((rtVal = cvt_file_n_save(argv[3], "-[]")) >= 0)
        {
            fprintf(stdout, "File Conversion Success. (%d Words Changed.)\n", rtVal);
            return 0;
        }
        else
        {
            fprintf(stdout, "File Conversion Failed.\n");
            return -1;
        }
    }
    else
    {
        if ((rtVal = cvt_str_n_print(argv[1], NULL)) >= 0)
        {
            fprintf(stdout, "String Conversion Success. (Returns: %d)\n", rtVal);
            return 0;
        }
        else
        {
            fprintf(stdout, "String Conversion Failed.\n");
            return -1;
        }
    }

    return 0;
}