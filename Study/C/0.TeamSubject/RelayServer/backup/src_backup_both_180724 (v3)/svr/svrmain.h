#ifndef __SVRMAIN_H__
#define __SVRMAIN_H__

#include "stdheader.h"

typedef struct _TOSocket {
	SOCKET hSoc;			/* 소켓 핸들 */
	long long dwConTime;	/* 접속시점 시간 */
	int msgId;				/* 메시지 고유 번호 */
} TOSocket;

int main(int argc, char** argv);
int checkArgc(int argc);
int serverWork();
int clrCliSocket(SOCKET hSocket, TOSocket *pSocket, int length);

#endif