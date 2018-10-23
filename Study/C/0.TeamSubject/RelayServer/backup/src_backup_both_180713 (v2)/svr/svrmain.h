#ifndef __SVRMAIN_H__
#define __SVRMAIN_H__

typedef struct _cliSocket {
	int id;
	SOCKET hSocket;				/* 소켓 핸들 */
	long long dwConTime;		/* 최초 접속 시간 */	
} cliSocket;

int main(int argc, char** argv);
int checkArgc(int argc);
int serverWork();

#endif