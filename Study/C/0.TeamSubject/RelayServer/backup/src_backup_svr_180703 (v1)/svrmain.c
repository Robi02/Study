#include "svrmain.h"
#include "stdheader.h"
#include "svrglobal.h"
#include "svrsocket.h"
#include "commonlib.h"

/*
 * [ Workflow ]
 * 1. ���α׷� ���� �� �ʱ�ȭ.
 * 2. �߰��������� ���μ����� ���� �ΰ� ����. (Select ���)
 * 3. Ŭ���̾�Ʈ���� �߰������� ������ ����.	----------------��
 * 4. ������ �����ư��� ���μ����� ����.					| -> ������� ȸ��(Record ����)��ŭ �ݺ�
 * 5. ������� ����� Ŭ���̾�Ʈ���� ������.	----------------��
 *    + ������ �������� ���� ����, Ŭ���̾�Ʈ ���۴�� ���� �ݾƼ� Ŭ�� ���� �˸�.
 * 6. �ۼ��� �α� �ۼ�
 *
 */

int main(int argc, char** argv) {
	int errorCode = 0;
	
	if (0 != checkArgc(argc))
	{
		fprintf(stderr, ".bat���Ϸ� ������Ѿ� �մϴ�.\n");
		return -1;
	}
	
	if (0 != initGlobalEnvs(argc, argv))
	{
		fprintf(stderr, "���� ȯ�溯�� ���� ����.\n");
		return -1;
	}
	
	if (0 != (errorCode = serverWork()))
	{
		fprintf(stderr, "���� ���� �߻�. (Code:%d)\n", errorCode);
		return -1;
	}
	
	return freeGlobalEnvs();
}

int checkArgc(int argc) {
	if (1 >= argc)
	{
		return -1;
	}
	
	return 0;
}

int serverWork() {
	int					MaxClients = FD_SETSIZE, SzBuf = 1030; /* FD_SETSIZE(64) */
	char				recvBuf[SzBuf];
	WSADATA				wsaData;
	SOCKET				tempSocket, hostSocket, svrSocket, cliSocket[MaxClients];
	struct sockaddr_in	svrAddr, hostAddr, cliAddr;
	int					i, selSocCnt, byteLen, recvLen, errCode;
	fd_set				readFds;
	
	fprintf(stdout, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	fprintf(stdout, "* ���� ���� ����...\n");
	
	/* �ʱ�ȭ */
	fprintf(stderr, "�ʱ�ȭ ����.\n");
	
	for (i = 0 ; i < MaxClients; ++i)
	{
		cliSocket[i] = 0;
	}
	
	fprintf(stderr, "cliSocket[%d] �ʱ�ȭ �Ϸ�.\n", MaxClients);
	
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		errCode = WSAGetLastError();
		fprintf(stderr, "WSAStartup() ����. (Code:%d)\n", errCode);
		return errCode;
	}
	
	fprintf(stderr, "WSAStartup() �Ϸ�.\n");
	
	/* ���� ���� */
	if (INVALID_SOCKET == (hostSocket = socket(AF_INET, SOCK_STREAM, 0)))
	{
		errCode = WSAGetLastError();
		fprintf(stderr, "socket() ���� ����. (Code:%d)\n", errCode);
		return errCode;
	}
	
	fprintf(stderr, "socket() ���� �Ϸ�.\n");
	
	/* ���ε� */
	hostAddr.sin_family = AF_INET;
	hostAddr.sin_addr.s_addr = INADDR_ANY;
	hostAddr.sin_port = htons(atoi(g_RELAY_PORT));
	
	if (SOCKET_ERROR == bind(hostSocket, (struct sockaddr*)&hostAddr, sizeof(hostAddr)))
	{
		errCode = WSAGetLastError();
		fprintf(stderr, "bind() ����. (Code:%d)\n", errCode);
		return errCode;
	}
	
	fprintf(stderr, "bind() �Ϸ�. (%s:%s)\n", INADDR_ANY, g_RELAY_PORT);
	
	/* ���� */
	if (SOCKET_ERROR == listen(hostSocket, 8))
	{
		errCode = WSAGetLastError();
		fprintf(stderr, "listen() ����. (Code:%d)\n", errCode);
		return errCode;
	}
	
	fprintf(stderr, "listen() �Ϸ�.\n");
	fprintf(stderr, "���� ���� ����.\n");
	
	while (1)
	{
		/* Clear the socket fd set */
		FD_ZERO(&readFds);
			
		/* Add host socket to fd set */
		FD_SET(hostSocket, &readFds);
		
		/* Add client sockets to fd set */
		for (i = 0; i < MaxClients; ++i)
		{
			tempSocket = cliSocket[i];
			
			if (tempSocket > 0)
			{
				FD_SET(tempSocket, &readFds);
			}
		}
		
		/* Wait for and activity on any of the sockets, timeout is NULL, so wait indefinitely */
		fprintf(stderr, "select() �����...\n");
		
		if (SOCKET_ERROR == (selSocCnt = select(0, &readFds, NULL, NULL, NULL)))
		{
			fprintf(stderr, "select() ����. (Code:%d)\n", WSAGetLastError());
			continue;
		}
		
		fprintf(stderr, "select() : %d\n", selSocCnt);
		
		/* If something happened on the host socket, then its an incoming connection */
		if (FD_ISSET(hostSocket, &readFds))
		{
			if (0 > (tempSocket = accept(hostSocket, (struct sockaddr*)&cliAddr, (int*)&byteLen)))
			{
				fprintf(stderr, "accept() ����. (Client:%s:%d, Code:%d)\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), tempSocket);
				continue;
			}
			
			for (i = 0; i < MaxClients; ++i)
			{
				if (cliSocket[i] == 0)
				{
					cliSocket[i] = tempSocket;
					fprintf(stderr, "Ŭ���̾�Ʈ(%s:%d:%d) ���� ����.\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i);
					break;
				}
			}
		}
		
		/* Else its some I/O operation on some other socket */
		for (i = 0; i < MaxClients; ++i)
		{
			tempSocket = cliSocket[i];
			
			/* If cli presend in read sockets */
			if (FD_ISSET(tempSocket, &readFds))
			{
				/* Get details of the client */
				getpeername(tempSocket, (struct sockaddr*)&cliAddr, (int*)&byteLen);
				
				/* Check if it was for closing, and also read the incoming msg
				   recv doesn't place a NULL terminator at the end of the string (whilst printf %s assumes there is one) */
				if (SOCKET_ERROR == (recvLen = recv(tempSocket, recvBuf, SzBuf, 0)))
				{
					errCode = WSAGetLastError();
					
					if (errCode == WSAECONNRESET)
					{
						/* Somebody disconnected, get his details and print */
						closesocket(tempSocket);
						cliSocket[i] = 0;
						fprintf(stderr, "Ŭ���̾�Ʈ(%s:%d:%d) ������ ���� ����.\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i);
						continue;
					}
					
					fprintf(stderr, "Ŭ���̾�Ʈ(%s:%d:%d) recv() ����. (Code:%d)\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i, errCode);
					continue;
				}
				else if (0 == recvLen)
				{
					/* Somebody disconnected, get his details and print */
					closesocket(tempSocket);
					cliSocket[i] = 0;
					fprintf(stderr, "Ŭ���̾�Ʈ(%s:%d:%d) ���������� ���� ����.\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i);
					continue;
				}
				else
				{
					/* Echo back test */
					recvBuf[recvLen] = '\0';
					fprintf(stderr, "\nrecv(%s)\n", recvBuf);
					send(tempSocket, recvBuf, recvLen, 0);
				}
			}
		}
	}
	
	closesocket(tempSocket);
	closesocket(hostSocket);
	WSACleanup();
	
	fprintf(stdout, "* ���� ���� ����.\n");
	fprintf(stdout, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	
	return 0;
}