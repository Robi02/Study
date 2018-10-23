#include "main.h"

int main(int argc, char **argv) {
	if (argc < 2)
	{
		fprintf(stderr, "소스파일을 드래그하여 프로그램을 실행하십시오.\n");
		return -1;
	}
	
	fprintf(stdout, "=================================================================\n");
	
	FILE *pCodeFile = NULL;
	char arLineBuf[1024];
	int totalLnCnt = 0;
	int lnCnt = 0;
	
	for (int i = 1; i < argc; ++i)
	{
		if ((pCodeFile = fopen(argv[i], "r")) == NULL)
		{
			continue;
		}
		
		lnCnt = 0;
		
		while (fgets(arLineBuf, sizeof(arLineBuf), pCodeFile))
		{
			++lnCnt;
		}
		
		fprintf(stdout, "* %d. File: '%s', Lines: %d\n", i, argv[i], lnCnt);
		totalLnCnt += lnCnt;
	}
	
	fprintf(stdout, "=================================================================\n");
	fprintf(stdout, "> TotalFiles: %d, TotalLines: %d\n", argc - 1, totalLnCnt);
	getch();
	
	return 0;
}