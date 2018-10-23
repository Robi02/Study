#ifndef __SVRGLOBAL_H__
#define __SVRGLOBAL_H__

#include "stdheader.h"

extern char *g_SERVER_IP;			/* 은행서버 IP */
extern char *g_SERVER_PORT;			/* 은행서버 PORT */
extern char *g_RELAY_PORT;			/* 중개서버 PORT (이 프로그램에서 사용할 포트) */
extern char *g_OUT_LOG_FILE_PATH;	/* 로그파일 경로 */

extern FILE *g_LOG;

int setStrEnv(char *pGvarName, char **pGvar, char *pData);
int setIntEnv(char *pGvarName, int *pGvar, int data);
int initGlobalEnvs(int argc, char **argv);
int freeGlobalEnvs();

#endif