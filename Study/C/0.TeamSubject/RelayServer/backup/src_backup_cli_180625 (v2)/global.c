#include "global.h"
#include "msgfileio.h"

char *g_FB_PARENT_COMP_NAME;
char *g_FB_PARENT_COMP_CODE;
char *g_FB_PARENT_BANK_CODE_2;
char *g_FB_PARENT_BANK_CODE_3;
char *g_FB_PARENT_ACCOUNT_NUMB;
char *g_FB_DEPOSIT_BANK_CODE_2;
char *g_FB_DEPOSIT_BANK_CODE_3;

char *g_SERVER_IP;
char *g_SERVER_PORT;
char *g_IN_MSG_FILE_PATH;
char *g_OUT_MSG_FILE_PATH;
char *g_OUT_LOG_FILE_PATH;

FILE *g_LOG;

int setEnv(char *pGvarName, char **pGvar, char *pData) {
	int szData = 0;
	
	if (pData == NULL)
	{
		fprintf(stderr, "주의! '%s'에 null을 대입하려 합니다. (pData == NULL)\n", pGvarName);
		return 0;
	}
	
	szData = strlen(pData) + 1;		/* +1 : null('\0')문자 포함 */
	*pGvar = (char*)malloc(szData);
	memcpy(*pGvar, pData, szData);
	
	return 0;
}

int initGlobalEnvs(int argc, char **argv) {
	setEnv("g_FB_PARENT_COMP_NAME", &g_FB_PARENT_COMP_NAME, argv[1]);
	setEnv("g_FB_PARENT_COMP_CODE", &g_FB_PARENT_COMP_CODE, argv[2]);
	setEnv("g_FB_PARENT_BANK_CODE_2", &g_FB_PARENT_BANK_CODE_2, argv[3]);
	setEnv("g_FB_PARENT_BANK_CODE_3", &g_FB_PARENT_BANK_CODE_3, argv[4]);
	setEnv("g_FB_PARENT_ACCOUNT_NUMB", &g_FB_PARENT_ACCOUNT_NUMB, argv[5]);
	setEnv("g_FB_DEPOSIT_BANK_CODE_2", &g_FB_DEPOSIT_BANK_CODE_2, argv[6]);
	setEnv("g_FB_DEPOSIT_BANK_CODE_3", &g_FB_DEPOSIT_BANK_CODE_3, argv[7]);
	
	setEnv("g_SERVER_IP", &g_SERVER_IP, argv[8]);
	setEnv("g_SERVER_PORT",  &g_SERVER_PORT, argv[9]); 
	setEnv("g_IN_MSG_FILE_PATH", &g_IN_MSG_FILE_PATH, argv[10]);
	setEnv("g_OUT_MSG_FILE_PATH", &g_OUT_MSG_FILE_PATH, argv[11]);
	setEnv("g_OUT_LOG_FILE_PATH", &g_OUT_LOG_FILE_PATH, argv[12]);
	
	return 0;
}

int freeGlobalEnvs() {
	free(g_FB_PARENT_COMP_NAME);
	free(g_FB_PARENT_COMP_CODE);
	free(g_FB_PARENT_BANK_CODE_2);
	free(g_FB_PARENT_BANK_CODE_3);
	free(g_FB_PARENT_ACCOUNT_NUMB);
	free(g_FB_DEPOSIT_BANK_CODE_2);
	free(g_FB_DEPOSIT_BANK_CODE_3);
	
	free(g_SERVER_IP);
    free(g_SERVER_PORT);
    free(g_IN_MSG_FILE_PATH);
    free(g_OUT_MSG_FILE_PATH);
    free(g_OUT_LOG_FILE_PATH);
	
	g_FB_PARENT_COMP_NAME = NULL;
	g_FB_PARENT_COMP_CODE = NULL;
	g_FB_PARENT_BANK_CODE_2 = NULL;
	g_FB_PARENT_BANK_CODE_3 = NULL;
	g_FB_PARENT_ACCOUNT_NUMB = NULL;
	g_FB_DEPOSIT_BANK_CODE_2 = NULL;
	g_FB_DEPOSIT_BANK_CODE_3 = NULL;
	
	g_SERVER_IP = NULL;
	g_SERVER_PORT = NULL;
	g_IN_MSG_FILE_PATH = NULL;
	g_OUT_MSG_FILE_PATH = NULL;
	g_OUT_LOG_FILE_PATH = NULL;
	
	return 0;
}