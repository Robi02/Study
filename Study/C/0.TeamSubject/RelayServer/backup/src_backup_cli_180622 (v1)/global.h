#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "stdheader.h"

extern char *g_FB_PARENT_COMP_NAME;
extern char *g_FB_PARENT_COMP_CODE;
extern char *g_FB_PARENT_BANK_CODE_2;
extern char *g_FB_PARENT_BANK_CODE_3;
extern char *g_FB_PARENT_ACCOUNT_NUMB;
extern char *g_FB_DEPOSIT_BANK_CODE_2;
extern char *g_FB_DEPOSIT_BANK_CODE_3;

extern char *g_SERVER_IP;
extern char *g_SERVER_PORT;
extern char *g_IN_MSG_FILE_PATH;
extern char *g_OUT_MSG_FILE_PATH;
extern char *g_OUT_LOG_FILE_PATH;

extern FILE *g_LOG;

int setEnv(char *pGvarName, char **pGvar, char *pData);
int initGlobals(int argc, char **argv);

#endif