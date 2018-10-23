#include "svrglobal.h"

char *g_SERVER_IP;		
char *g_SERVER_PORT;		
char *g_RELAY_PORT;		
char *g_OUT_LOG_FILE_PATH;

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
	setStrEnv("g_SERVER_IP",			&g_SERVER_IP,			argv[1]);
	setStrEnv("g_SERVER_PORT",			&g_SERVER_PORT,			argv[2]);
	setStrEnv("g_RELAY_PORT",			&g_RELAY_PORT,			argv[3]);
	setStrEnv("g_OUT_LOG_FILE_PATH",	&g_OUT_LOG_FILE_PATH,	argv[4]);
	
	/* ----------------------------------------------------------------*/
	
	return 0;
}

int freeGlobalEnvs() {
	free(g_SERVER_IP);
	free(g_SERVER_PORT);
	free(g_RELAY_PORT);
	free(g_OUT_LOG_FILE_PATH);
	
	/* ----------------------------------------------------------------*/
	
	g_SERVER_IP = NULL;
	g_SERVER_PORT = NULL;
	g_RELAY_PORT = NULL;
	g_OUT_LOG_FILE_PATH = NULL;
	
	return 0;
}