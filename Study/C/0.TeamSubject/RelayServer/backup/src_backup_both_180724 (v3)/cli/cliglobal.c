#include "cliglobal.h"
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
char *g_REUSABLE_SOCKET;	/* "Y" or "N" */

int g_RECONNECT_TRY_CNT;
int g_AVG_SEND_SPEED_PER_SEC;
int g_MAX_SEND_SPEED_PER_SEC;

FILE *g_LOG;

int setStrEnv(char *pGvarName, char **pGvar, char *pData) {
	int szData = 0;
	
	if (pData == NULL)
	{
		fprintf(stderr, "주의! '%s'에 null을 대입하려 합니다. (pData == NULL)\n", pGvarName);
		*pGvar = NULL;
		return 0;
	}
	
	szData = strlen(pData) + 1;		/* +1 : null('\0')문자 포함 */
	*pGvar = (char*)malloc(szData);
	memcpy(*pGvar, pData, szData);
	
	return 0;
}

int setIntEnv(char *pGvarName, int *pGvar, int data) {
	if (pGvar == NULL)
	{
		fprintf(stderr, "오류. (pGvar == NULL)\n");
		return -1;
	}
	
	*pGvar = data;
	
	return 0;
}

int initGlobalEnvs(int argc, char **argv) {
	setStrEnv("g_FB_PARENT_COMP_NAME", &g_FB_PARENT_COMP_NAME, argv[1]);
	setStrEnv("g_FB_PARENT_COMP_CODE", &g_FB_PARENT_COMP_CODE, argv[2]);
	setStrEnv("g_FB_PARENT_BANK_CODE_2", &g_FB_PARENT_BANK_CODE_2, argv[3]);
	setStrEnv("g_FB_PARENT_BANK_CODE_3", &g_FB_PARENT_BANK_CODE_3, argv[4]);
	setStrEnv("g_FB_PARENT_ACCOUNT_NUMB", &g_FB_PARENT_ACCOUNT_NUMB, argv[5]);
	setStrEnv("g_FB_DEPOSIT_BANK_CODE_2", &g_FB_DEPOSIT_BANK_CODE_2, argv[6]);
	setStrEnv("g_FB_DEPOSIT_BANK_CODE_3", &g_FB_DEPOSIT_BANK_CODE_3, argv[7]);
	
	setStrEnv("g_SERVER_IP", &g_SERVER_IP, argv[8]);
	setStrEnv("g_SERVER_PORT",  &g_SERVER_PORT, argv[9]); 
	setStrEnv("g_IN_MSG_FILE_PATH", &g_IN_MSG_FILE_PATH, argv[10]);
	setStrEnv("g_OUT_MSG_FILE_PATH", &g_OUT_MSG_FILE_PATH, argv[11]);
	setStrEnv("g_OUT_LOG_FILE_PATH", &g_OUT_LOG_FILE_PATH, argv[12]);
	setStrEnv("g_REUSABLE_SOCKET", &g_REUSABLE_SOCKET, argv[13]);
	
	/* ----------------------------------------------------------------*/
	
	setIntEnv("g_RECONNECT_TRY_CNT", &g_RECONNECT_TRY_CNT, atoi(argv[14]));
	setIntEnv("g_AVG_SEND_SPEED_PER_SEC", &g_AVG_SEND_SPEED_PER_SEC, atoi(argv[15]));
	setIntEnv("g_MAX_SEND_SPEED_PER_SEC", &g_MAX_SEND_SPEED_PER_SEC, atoi(argv[16]));
	
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
	free(g_REUSABLE_SOCKET);
	
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
	g_REUSABLE_SOCKET = NULL;
	
	/* ----------------------------------------------------------------*/
	
	g_RECONNECT_TRY_CNT = 0;
	g_AVG_SEND_SPEED_PER_SEC = 0;
	g_MAX_SEND_SPEED_PER_SEC = 0;
	
	return 0;
}