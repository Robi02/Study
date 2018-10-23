#include "svrmain.h"
#include "stdheader.h"
#include "svrglobal.h"
#include "svrsocket.h"
#include "msgfileio.h"
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
 
int clrSocket(SOCKET hSocket, SOCKET *pSocket, int length);

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
	int					MaxClients = FD_SETSIZE / 2, MaxServers = 2, SzBuf = 512; /* FD_SETSIZE(64) */
	char				cliRecvBuf[512]; /* SzBuf */
	char				svrRecvBuf[512]; /* SzBuf */
	WSADATA				wsaData;
	SOCKET				tempCliSocket, tempSvrSocket, hostSocket, svrSocket[2], /* MaxServers */ cliSocket[32]; /* MaxClients */
	SOCKET				cliSvrMapping[2]; /* MaxServers */
	struct sockaddr_in	svrAddr, hostAddr, cliAddr;
	int					i, j, selSocCnt, byteLen, cliRecvLen, svrRecvLen, errCode, curSessionIdx = 0;
	fd_set				readFds;
	
	fprintf(stdout, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	fprintf(stdout, "* 서버 구동 시작...\n");
	
	/* Make log file */
	g_LOG = openMsgFile(g_OUT_LOG_FILE_PATH, "w");
	fprintf(stderr, "로그 생성 완료.\n");
	
	/* Init */
	fprintf(stderr, "초기화 시작.\n");
	
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_addr.s_addr = inet_addr(g_SERVER_IP);
	svrAddr.sin_port = htons(atoi(g_SERVER_PORT));
	
	fprintf(stderr, "svrAddr 초기화 완료. (%s:%s)\n", g_SERVER_IP, g_SERVER_PORT);
	
	for (i = 0; i < MaxClients; ++i)
	{
		cliSocket[i] = 0;
	}
	
	for (i = 0; i < MaxServers; ++i)
	{
		svrSocket[i] = 0;
		cliSvrMapping[i] = 0;
	}
	
	fprintf(stderr, "cli/SvrSocket[%d/%d] 초기화 완료.\n", MaxClients, MaxServers);
	
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		errCode = WSAGetLastError();
		fprintf(stderr, "WSAStartup() 오류. (Code:%d)\n", errCode);
		return errCode;
	}
	
	fprintf(stderr, "WSAStartup() 완료.\n");
	
	/* Create socket */
	if (INVALID_SOCKET == (hostSocket = socket(AF_INET, SOCK_STREAM, 0)))
	{
		errCode = WSAGetLastError();
		fprintf(stderr, "socket() 생성 오류. (Code:%d)\n", errCode);
		return errCode;
	}
	
	fprintf(stderr, "socket() 생성 완료.\n");
	
	/* Binding */
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
	
	/* Listen */
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
			
		/* Add host and server socket to fd set */
		FD_SET(hostSocket, &readFds);
		
		for (i = 0; i < MaxServers; ++i)
		{
			if (svrSocket[i] != 0)
			{
				FD_SET(svrSocket[i], &readFds);
			}
		}
		
		/* Add client sockets to fd set */
		for (i = 0; i < MaxClients; ++i)
		{
			tempCliSocket = cliSocket[i];
			
			if (tempCliSocket > 0)
			{
				FD_SET(tempCliSocket, &readFds);
			}
		}
		
		/* Wait for and activity on any of the sockets, timeout is NULL, so wait indefinitely */
		/*fprintf(stderr, "select() 대기중...\n"); */
		
		if (SOCKET_ERROR == (selSocCnt = select(0, &readFds, NULL, NULL, NULL)))
		{
			fprintf(stderr, "select() 오류. (Code:%d)\n", WSAGetLastError());
			continue;
		}
		else if (0 == selSocCnt)
		{
			continue;
		}
		
		/* fprintf(stderr, "select() : %d\n", selSocCnt); */
		
		/* If something happened on the host socket, then its an incoming connection */
		if (FD_ISSET(hostSocket, &readFds))
		{
			if (0 > (tempCliSocket = accept(hostSocket, (struct sockaddr*)&cliAddr, (int*)&byteLen)))
			{
				fprintf(stderr, "accept() 오류. (Client:%s:%d, Code:%d)\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), tempCliSocket);
				continue;
			}
			
			for (i = 0; i < MaxClients; ++i)
			{
				if (cliSocket[i] == 0)
				{
					cliSocket[i] = tempCliSocket;
					fprintf(stderr, "클라이언트(%s:%d:%d) 접속시킴.\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i);
					break;
				}
			}
			
			if (MaxClients == i)
			{
				closesocket(tempCliSocket);
				fprintf(stderr, "클라이언트(%s:%d:%d) 접속 종료시킴. (최대 클라이언트 수 도달)\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i);
			}
		}
		
		/* Read operation on client socket */
		for (i = 0; i < MaxClients; ++i)
		{
			tempCliSocket = cliSocket[i];
			
			/* If cli presend in read sockets */
			if (FD_ISSET(tempCliSocket, &readFds))
			{
				--selSocCnt;
				
				/* Check svr mapping is empty */
				if (cliSvrMapping[curSessionIdx] != 0)
				{
					continue; /* If cliSvrMapping is full, try next time */
				}
				
				/* Get details of the client */
				getpeername(tempCliSocket, (struct sockaddr*)&cliAddr, (int*)&byteLen);
				
				/* Check if it was for closing, and also read the incoming msg
				   recv doesn't place a NULL terminator at the end of the string (whilst printf %s assumes there is one) */
				if (SOCKET_ERROR == (cliRecvLen = recv(tempCliSocket, cliRecvBuf, SzBuf, 0)))
				{
					errCode = WSAGetLastError();
					
					if (WSAECONNRESET == errCode)
					{
						/* Client disconnected. init client array, get socket detail and print */
						cliSocket[i] = 0;
						fprintf(stderr, "클라이언트(%s:%d:%d) 오류로 접속 종료.\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i);
						continue;
					}
					
					fprintf(stderr, "클라이언트(%s:%d:%d) recv() 실패. (Code:%d)\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i, errCode);
					continue;
				}
				else if (0 == cliRecvLen)
				{
					/* Client disconnected. init client array, get socket detail and print */
					cliSocket[i] = 0;
					fprintf(stderr, "클라이언트(%s:%d:%d) 정상적으로 접속 종료.\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i);
					continue;
				}
				else
				{
					/* Cli recv logging */
					writeLog(g_LOG, cliRecvBuf, cliRecvLen, "rcvcli(", ")");
					
					/* Send data to server */
					tempSvrSocket = svrSocket[curSessionIdx];
					
					/* Mapping curSessionIdx and cliSocket */
					cliSvrMapping[curSessionIdx] = tempCliSocket;
					
					while (SOCKET_ERROR == send(tempSvrSocket, cliRecvBuf, cliRecvLen, 0))
					{
						fprintf(stderr, "서버 소캣(%s:%d) 재생성 시도. (Session:%d)\n", inet_ntoa(svrAddr.sin_addr), ntohs(svrAddr.sin_port), curSessionIdx);
						
						while (INVALID_SOCKET == (tempSvrSocket = socket(PF_INET, SOCK_STREAM, 0)))
						{
							fprintf(stderr, "서버 소캣(%s:%d) 생성 오류. (Session:%d)\n", inet_ntoa(svrAddr.sin_addr), ntohs(svrAddr.sin_port), curSessionIdx);
							Sleep(100);
						}
						
						while (SOCKET_ERROR == connect(tempSvrSocket, (struct sockaddr*)&svrAddr, sizeof(svrAddr)))
						{
							fprintf(stderr, "서버(%s:%d) 연결 오류. (Session:%d)\n", inet_ntoa(svrAddr.sin_addr), ntohs(svrAddr.sin_port), curSessionIdx);
							Sleep(100);
						}
						
						svrSocket[curSessionIdx] = tempSvrSocket;
						fprintf(stderr, "서버 소캣(%s:%d) 재생성 성공. (Session:%d)\n", inet_ntoa(svrAddr.sin_addr), ntohs(svrAddr.sin_port), curSessionIdx);
					}
					
					/* Svr send logging */
					writeLog(g_LOG, cliRecvBuf, cliRecvLen, "sndsvr(", ")");
					
					/* Set next session */
					curSessionIdx = (++curSessionIdx) % MaxServers;
				}
			}
			
			if (selSocCnt <= 0) break;
		}
		
		/* Read operation on Server socket */
		for (i = 0; i < MaxServers; ++i)
		{
			tempSvrSocket = svrSocket[i];
			tempCliSocket = cliSvrMapping[i];
			
			/* If svr presend in read sockets */
			if (FD_ISSET(tempSvrSocket, &readFds))
			{
				--selSocCnt;
				
				if (SOCKET_ERROR == (svrRecvLen = recv(tempSvrSocket, svrRecvBuf, SzBuf, 0)))
				{
					errCode = WSAGetLastError();
					
					if (errCode == WSAECONNRESET)
					{
						closesocket(tempCliSocket);
						clrSocket(tempCliSocket, cliSocket, MaxClients);
						svrSocket[i] = 0;
						cliSvrMapping[i] = 0;
						fprintf(stderr, "서버(%s:%d)와 연결이 비정상 종료됨. (Session:%d)\n", inet_ntoa(svrAddr.sin_addr), ntohs(svrAddr.sin_port), curSessionIdx);
						writeLog(g_LOG, NULL, 0, "rcvsvr(socket closed by error", ")");
						continue;
					}
				}
				else if (0 == svrRecvLen)
				{
					closesocket(tempCliSocket);
					clrSocket(tempCliSocket, cliSocket, MaxClients);
					svrSocket[i] = 0;
					cliSvrMapping[i] = 0;
					fprintf(stderr, "서버(%s:%d)에서 연결 종료시킴. (Session:%d)\n", inet_ntoa(svrAddr.sin_addr), ntohs(svrAddr.sin_port), curSessionIdx);
					writeLog(g_LOG, NULL, 0, "rcvsvr(socket closed by server", ")");
					continue;
				}
				else
				{
					/* Get cliSocket from mapping array and initialize */
					cliSvrMapping[i] = 0;
					
					/* Svr recv logging */
					writeLog(g_LOG, svrRecvBuf, svrRecvLen, "rcvsvr(", ")");
					
					/* Send to client */
					send(tempCliSocket, svrRecvBuf, svrRecvLen, 0);
					
					/* Remove client */
					closesocket(tempCliSocket);
					clrSocket(tempCliSocket, cliSocket, MaxClients);
					
					fprintf(stderr, "클라이언트(%s:%d:%d) 접속 종료시킴.\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i);
					
					/* Cli send logging */
					writeLog(g_LOG, svrRecvBuf, svrRecvLen, "clisnd(", ")");
				}
			}
			
			if (selSocCnt <= 0) break;
		}
	}
	
	/* Close client sockets */
	for (i = 0; i < MaxClients; ++i)
	{
		if (cliSocket[i] != 0)
		{
			closesocket(cliSocket[i]);
			cliSocket[i] = 0;
		}
	}
	
	/* Close server sockets */
	for (i = 0; i < MaxServers; ++i)
	{
		if (svrSocket[i] != 0)
		{
			closesocket(svrSocket[i]);
			svrSocket[i] = 0;
		}
	}
	
	/* Close host socket */
	closesocket(hostSocket);
	WSACleanup();
	
	fprintf(stdout, "* 서버 구동 종료.\n");
	fprintf(stdout, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	
	return 0;
}

int clrSocket(SOCKET hSocket, SOCKET *pSocket, int length) {
	int i = 0;
	
	for (i = 0; i < length; ++i)
	{
		if (pSocket[i] == hSocket)
		{
			pSocket[i] = 0;
			return 0;
		}
	}
	
	return -1;
}