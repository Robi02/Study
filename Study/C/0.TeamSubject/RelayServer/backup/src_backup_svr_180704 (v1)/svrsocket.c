#include "svrsocket.h"
#include "svrglobal.h"
#include "msgfileio.h"
#include "commonlib.h"

static char STX = 0x2;
static char ETX = 0x3;
static int DataMinSz = 0;
static int DataMaxSz = 1024;
static int PacketMinSz = 6;
static int PacketMaxSz = 1030; /* (PacketMinSz + DataMinSz) */

int connectSocket(SOCKET *pSocket, char *pIP, int port) {
	WSADATA stWsaData;
	SOCKADDR_IN stSvrAddr;
	
	if (WSAStartup(MAKEWORD(2, 2), &stWsaData) != 0) /* 2.2 version */
	{
		fprintf(stderr, "WSAStartup() 神嫌.\n");
		return -1;
	}
	
	if ((*pSocket = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		fprintf(stderr, "社長 持失 神嫌.\n");
		return -1;
	}
	
	memset(&stSvrAddr, 0, sizeof(stSvrAddr));
	stSvrAddr.sin_family = AF_INET;
	stSvrAddr.sin_addr.s_addr = inet_addr(pIP);
	stSvrAddr.sin_port = htons(port);
	
	if (connect(*pSocket, (SOCKADDR*)&stSvrAddr, sizeof(stSvrAddr)) == SOCKET_ERROR)
	{
		return -1;
	}
}

int closeSocket(SOCKET *pSocket) {
	closesocket(*pSocket);
	WSACleanup();
	return 0;
}