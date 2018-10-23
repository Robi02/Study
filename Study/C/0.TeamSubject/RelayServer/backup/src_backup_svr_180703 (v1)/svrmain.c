#include "svrmain.h"
#include "stdheader.h"
#include "svrglobal.h"
#include "svrsocket.h"
#include "commonlib.h"

/*
 * [ Workflow ]
 * 1. 프로그램 구동 및 초기화.
 * 2. 중개서버에서 메인서버로 세션 두개 연결. (Select 사용)
 * 3. 클라이언트에서 중개서버로 데이터 전송.	----------------┐
 * 4. 세션을 번갈아가며 메인서버로 전송.					| -> 응답받은 회수(Record 개수)만큼 반복
 * 5. 응답받은 결과를 클라이언트에게 돌려줌.	----------------┘
 *    + 연결이 끊어지면 연결 복원, 클라이언트 전송대기 소켓 닫아서 클라에 오류 알림.
 * 6. 송수신 로그 작성
 *
 */

int main(int argc, char** argv) {
	int errorCode = 0;
	
	if (0 != checkArgc(argc))
	{
		fprintf(stderr, ".bat파일로 실행시켜야 합니다.\n");
		return -1;
	}
	
	if (0 != initGlobalEnvs(argc, argv))
	{
		fprintf(stderr, "전역 환경변수 저장 실패.\n");
		return -1;
	}
	
	if (0 != (errorCode = serverWork()))
	{
		fprintf(stderr, "서버 오류 발생. (Code:%d)\n", errorCode);
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
	fprintf(stdout, "* 서버 구동 시작...\n");
	
	/* 초기화 */
	fprintf(stderr, "초기화 시작.\n");
	
	for (i = 0 ; i < MaxClients; ++i)
	{
		cliSocket[i] = 0;
	}
	
	fprintf(stderr, "cliSocket[%d] 초기화 완료.\n", MaxClients);
	
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		errCode = WSAGetLastError();
		fprintf(stderr, "WSAStartup() 오류. (Code:%d)\n", errCode);
		return errCode;
	}
	
	fprintf(stderr, "WSAStartup() 완료.\n");
	
	/* 소켓 생성 */
	if (INVALID_SOCKET == (hostSocket = socket(AF_INET, SOCK_STREAM, 0)))
	{
		errCode = WSAGetLastError();
		fprintf(stderr, "socket() 생성 오류. (Code:%d)\n", errCode);
		return errCode;
	}
	
	fprintf(stderr, "socket() 생성 완료.\n");
	
	/* 바인딩 */
	hostAddr.sin_family = AF_INET;
	hostAddr.sin_addr.s_addr = INADDR_ANY;
	hostAddr.sin_port = htons(atoi(g_RELAY_PORT));
	
	if (SOCKET_ERROR == bind(hostSocket, (struct sockaddr*)&hostAddr, sizeof(hostAddr)))
	{
		errCode = WSAGetLastError();
		fprintf(stderr, "bind() 오류. (Code:%d)\n", errCode);
		return errCode;
	}
	
	fprintf(stderr, "bind() 완료. (%s:%s)\n", INADDR_ANY, g_RELAY_PORT);
	
	/* 리슨 */
	if (SOCKET_ERROR == listen(hostSocket, 8))
	{
		errCode = WSAGetLastError();
		fprintf(stderr, "listen() 오류. (Code:%d)\n", errCode);
		return errCode;
	}
	
	fprintf(stderr, "listen() 완료.\n");
	fprintf(stderr, "메인 루프 시작.\n");
	
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
		fprintf(stderr, "select() 대기중...\n");
		
		if (SOCKET_ERROR == (selSocCnt = select(0, &readFds, NULL, NULL, NULL)))
		{
			fprintf(stderr, "select() 오류. (Code:%d)\n", WSAGetLastError());
			continue;
		}
		
		fprintf(stderr, "select() : %d\n", selSocCnt);
		
		/* If something happened on the host socket, then its an incoming connection */
		if (FD_ISSET(hostSocket, &readFds))
		{
			if (0 > (tempSocket = accept(hostSocket, (struct sockaddr*)&cliAddr, (int*)&byteLen)))
			{
				fprintf(stderr, "accept() 오류. (Client:%s:%d, Code:%d)\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), tempSocket);
				continue;
			}
			
			for (i = 0; i < MaxClients; ++i)
			{
				if (cliSocket[i] == 0)
				{
					cliSocket[i] = tempSocket;
					fprintf(stderr, "클라이언트(%s:%d:%d) 접속 성공.\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i);
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
						fprintf(stderr, "클라이언트(%s:%d:%d) 오류로 접속 종료.\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i);
						continue;
					}
					
					fprintf(stderr, "클라이언트(%s:%d:%d) recv() 실패. (Code:%d)\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i, errCode);
					continue;
				}
				else if (0 == recvLen)
				{
					/* Somebody disconnected, get his details and print */
					closesocket(tempSocket);
					cliSocket[i] = 0;
					fprintf(stderr, "클라이언트(%s:%d:%d) 정상적으로 접속 종료.\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i);
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
	
	fprintf(stdout, "* 서버 구동 종료.\n");
	fprintf(stdout, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	
	return 0;
}