#ifndef __SVRMAIN_H__
#define __SVRMAIN_H__

typedef struct _cliSocket {
	int id;
	SOCKET hSocket;				/* ���� �ڵ� */
	long long dwConTime;		/* ���� ���� �ð� */	
} cliSocket;

int main(int argc, char** argv);
int checkArgc(int argc);
int serverWork();

#endif