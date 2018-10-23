#include "msgsocket.h"
#include "global.h"
#include "msgfileio.h"

static char STX = 0x2;
static char ETX = 0x3;
static int DataMinSz = 0;
static int DataMaxSz = 1024;
static int PacketMinSz = 6;
static int PacketMaxSz = 1030; /* (PacketMinSz + DataMinSz) */

int sendAndRecv(SOCKET *pSocket, SvrRecord *pstSvrRecord, int option) {
	int serverPort = atoi(g_SERVER_PORT);
	int szPacket = sizeof(SvrRecord) + PacketMinSz;
	char arBuf[1030] = { 0, };
	int szBuf = sizeof(arBuf);
	
	if (option == 0) /* ��ȸ�� ��Ĺ */
	{
		if (connectSocket(pSocket, g_SERVER_IP, serverPort) != 0)
		{
			return -1;
		}

		makePacketFromMsg(arBuf, szBuf, (char*)pstSvrRecord, sizeof(SvrRecord)); /* 300 -> 306 */		
		send(*pSocket, arBuf, szPacket, 0);
		writeLog(g_LOG, arBuf, szPacket, "snd(", ")");
		
		recv(*pSocket, arBuf, szPacket, 0);
		writeLog(g_LOG, arBuf, szPacket, "rcv(", ")");
		makeMsgFromPacket((char*)pstSvrRecord, sizeof(SvrRecord), arBuf, szPacket); /* 306 -> 300 */
		
		return closeSocket(pSocket);
	}
	else if (option == 1) /* ���� ��Ĺ */
	{
		/* recv == 0 => socket closed */
		return 0;
	}
	
	return -1;
}

int connectSocket(SOCKET *pSocket, char *pIP, int port) {
	WSADATA stWsaData;
	SOCKADDR_IN stSvrAddr;
	
	if (WSAStartup(MAKEWORD(2, 2), &stWsaData) != 0) /* 2.2 version */
	{
		fprintf(stderr, "WSAStartup() ����.\n");
		return -1;
	}
	
	if ((*pSocket = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		fprintf(stderr, "��Ĺ ���� ����.\n");
		return -1;
	}
	
	memset(&stSvrAddr, 0, sizeof(stSvrAddr));
	stSvrAddr.sin_family = AF_INET;
	stSvrAddr.sin_addr.s_addr = inet_addr(pIP);
	stSvrAddr.sin_port = htons(port);
	
	if (connect(*pSocket, (SOCKADDR*)&stSvrAddr, sizeof(stSvrAddr)) == SOCKET_ERROR)
	{
		fprintf(stderr, "connect() ����.\n");
		return -1;
	}
}

int closeSocket(SOCKET *pSocket) {
	closesocket(*pSocket);
	WSACleanup();
	return 0;
}

/*
 * [ Packet Format ]
 * > 000512345
 * > Size(4)/(1)/Data(0~1024)/(1)
 * > MinSz:6byte, MaxSz:1030, MinDataSz:0, MaxDataSz:1024
 */

char* makePacketFromMsg(char *pBuf, int szBuf, char *pMsg, int szMsg) {
	if (szBuf < szMsg)
	{
		fprintf(stderr, "pMsg/pBuf ũ�� ����. (szBuf:%d < szMsg:%d)\n", szBuf, szMsg);
		return NULL;
	}
	
	if (szMsg > DataMaxSz)
	{
		fprintf(stderr, "pMsg ũ�� ����. (szMsg:%d > %d)\n", szMsg, DataMaxSz);
		return NULL;
	}
	
	sprintf(pBuf, "%04d%c", szMsg + 2, STX); /* szMsg + strlen(STX + ETX) */
	memcpy(pBuf + 5, pMsg, min(szBuf, szMsg));
	pBuf[szMsg + 5] = ETX;
	
	return pBuf;
}

char* makeMsgFromPacket(char *pBuf, int szBuf, char *pPacket, int szPacket) {
	char arPacketSize[5] = { 0, };
	int szMsg = 0;
	
	if (szBuf + PacketMinSz < szPacket)
	{
		fprintf(stderr, "pMsg/szPacket ũ�� ����. (szBuf:%d < szPacket:%d)\n", szBuf, szPacket);
		return NULL;
	}

	strncpy(arPacketSize, pPacket, 4);
	szMsg = atoi(arPacketSize);
	memcpy(pBuf, pPacket + 5, min(szBuf, szMsg));	
	
	return pBuf;
}