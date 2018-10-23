#ifndef __SVRGLOBAL_H__
#define __SVRGLOBAL_H__

extern char *g_SERVER_IP;			/* ���༭�� IP */
extern char *g_SERVER_PORT;			/* ���༭�� PORT */
extern char *g_RELAY_PORT;			/* �߰����� PORT (�� ���α׷����� ����� ��Ʈ) */
extern char *g_OUT_LOG_FILE_PATH;	/* �α����� ��� */

int setStrEnv(char *pGvarName, char **pGvar, char *pData);
int setIntEnv(char *pGvarName, int *pGvar, int data);
int initGlobalEnvs(int argc, char **argv);
int freeGlobalEnvs();

#endif