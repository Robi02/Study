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
 * [ 문제 해결 과정 ]
 * 1. 중개서버를 통한 클라이언트 - 서버 매핑 구현에 몇가지 제약이 있다.
 *  1) 클라이언트 소켓 핸들로 매핑을 하는 경우 - 서버에서 데이터를 수신했을 시 해당 소켓 핸들이 닫혀있는 경우 혹은 Critical, OS에서 해당 소켓 번호를 재사용하여 
 *     다른 클라이언트가 해당 소켓을 사용하게 된다면 비정상적 응답 발생.
 *  2) 전문 번호로 매핑을 하는 경우 - 서로 다른 클라이언트의 전문이 같은 전문 번호를 가진다면 올바른 클라이언트에게 응답할 수 없다.
 *  
 * => 고유 번호를 전문에 삽입하여 고유 번호와 클라이언트를 매핑하는 방안이 제일 안전해 보인다.
 * => 서버와 연결이 소실되어 응답받지 못하는 클라이언트 배열이 가득 차서 다른 클라이언트를 받지 못할 경우를 대비해서,
 *    클라이언트 소켓에 타임아웃으로 2000ms 경과 시 끊어버리도록 한다.
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
	int					MaxClients = FD_SETSIZE / 2, MaxServers = 2, SzBuf = 512; /* FD_SETSIZE(64) */
	static int			msgId = 1;
	char				aryBuf[128];
	char				cliRecvBuf[512]; /* [SzBuf] */
	char				svrRecvBuf[512]; /* [SzBuf] */
	WSADATA				wsaData;
	SOCKET				tempCliSocket, tempSvrSocket, hostSocket, svrSocket[2]; /* [MaxServers] */
	TOSocket			cliSocket[32]; /* [MaxClients] */ /* TimeOut Socket */
	int					idx;
	struct sockaddr_in	svrAddr, hostAddr, cliAddr;
	int					i, j, selSocCnt, byteLen, cliRecvLen, svrRecvLen, errCode, curSessionIdx = 0;
	fd_set				readFds;
	struct timeval		selTimeout;
	long long			dwCurTime;
	
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
		cliSocket[i].hSoc = 0;
		cliSocket[i].dwConTime = 0;
		cliSocket[i].msgId = 0;
	}
	
	for (i = 0; i < MaxServers; ++i)
	{
		svrSocket[i] = 0;
	}
	
	fprintf(stderr, "cli/SvrSocket[%d/%d] 초기화 완료.\n", MaxClients, MaxServers);
	
	/* Select blocking timeout */
	selTimeout.tv_sec = 2;
    selTimeout.tv_usec = 0;
	fprintf(stderr, "selectTimeout 초기화 완료. (tv_sec:%ld, tv_usec:%ld)\n", selTimeout.tv_sec, selTimeout.tv_usec);
	
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
		dwCurTime = currentTimeMillis();
		
		for (i = 0; i < MaxClients; ++i)
		{
			tempCliSocket = cliSocket[i].hSoc;
			
			if (tempCliSocket > 0)
			{
				/* Check client TOSocket timeout */
				long long dwTimeDt = dwCurTime - (cliSocket[i].dwConTime + (long long)(g_CLI_SOC_TIMEOUT_DELAY));					
				
				if (dwTimeDt >= 0)
				{
					closesocket(cliSocket[i].hSoc);
					cliSocket[i].hSoc = 0;
					cliSocket[i].dwConTime = 0;
					fprintf(stderr, "클라이언트(%s:%d:%d) 타임아웃으로 접속종료시킴. (%lldms)\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i, (long long)g_CLI_SOC_TIMEOUT_DELAY + dwTimeDt);
					continue;
				}

				FD_SET(tempCliSocket, &readFds);
			}			
		}
		
		/* Wait for and activity on any of the sockets, timeout is NULL, so wait indefinitely */
		/*fprintf(stderr, "select() 대기중...\n"); */
		
		if (SOCKET_ERROR == (selSocCnt = select(0, &readFds, NULL, NULL, &selTimeout)))
		{
			fprintf(stderr, "select() 오류. (Code:%d)\n", WSAGetLastError());
			continue;
		}
		else if (0 == selSocCnt)
		{
			fprintf(stderr, "select() 대기중...\n");
			continue;
		}
		
		/* fprintf(stderr, "select() : %d\n", selSocCnt); */
		
		/* If something happened on the host socket, then its an incoming connection */
		if (FD_ISSET(hostSocket, &readFds))
		{
			--selSocCnt;
			
			if (0 > (tempCliSocket = accept(hostSocket, (struct sockaddr*)&cliAddr, (int*)&byteLen)))
			{
				fprintf(stderr, "accept() 오류. (Client:%s:%d, Code:%d)\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), tempCliSocket);
				continue;
			}
			
			for (i = 0; i < MaxClients; ++i)
			{
				if (cliSocket[i].hSoc == 0)
				{
					getpeername(cliSocket[i].hSoc, (struct sockaddr*)&cliAddr, (int*)&byteLen);
					cliSocket[i].hSoc = tempCliSocket;
					cliSocket[i].dwConTime = currentTimeMillis();
					cliSocket[i].msgId = 0;
					fprintf(stderr, "클라이언트(%s:%d) 접속시킴. (%d/%d)\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i + 1, MaxClients);
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
			tempCliSocket = cliSocket[i].hSoc;
			
			/* If cli presend in read sockets */
			if (FD_ISSET(tempCliSocket, &readFds))
			{
				--selSocCnt;
				
				/* Get details of the client */
				getpeername(tempCliSocket, (struct sockaddr*)&cliAddr, (int*)&byteLen);
				
				/* Check if it was for closing, and also read the incoming msg
				   recv doesn't place a NULL terminator at the end of the string (whilst printf %s assumes there is one) */
				memset(cliRecvBuf, 0, SzBuf);
				   
				if (SOCKET_ERROR == (cliRecvLen = recv(tempCliSocket, cliRecvBuf, SzBuf, 0)))
				{
					errCode = WSAGetLastError();
					
					if (WSAECONNRESET == errCode)
					{
						/* Client disconnected. init client array, get socket detail and print */
						cliSocket[i].hSoc = 0;
						cliSocket[i].dwConTime = 0;
						fprintf(stderr, "클라이언트(%s:%d:%d) 오류로 접속 종료.\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i);
						continue;
					}
					
					fprintf(stderr, "클라이언트(%s:%d:%d) recv() 실패. (Code:%d)\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i, errCode);
					continue;
				}
				else if (0 == cliRecvLen)
				{
					/* Client disconnected. init client array, get socket detail and print */
					cliSocket[i].hSoc = 0;
					cliSocket[i].dwConTime = 0;
					fprintf(stderr, "클라이언트(%s:%d:%d) 정상적으로 접속 종료.\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), i);
					continue;
				}
				else
				{
					/* Cli recv logging */
					writeLog(g_LOG, cliRecvBuf, cliRecvLen, "rcvcli(", ")");
					
					/* Send data to server */
					tempSvrSocket = svrSocket[curSessionIdx];
					
					/* Add client msgId into send packet's {spare byte(10)} */
					memset(aryBuf, 0x00, sizeof(aryBuf));
					sprintf(aryBuf, "%010d", msgId++);
					memcpy(cliRecvBuf + 285, aryBuf, 10);
					cliSocket[i].msgId = atoi(aryBuf);
					
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
			
			/* If svr presend in read sockets */
			if (FD_ISSET(tempSvrSocket, &readFds))
			{
				--selSocCnt;
				
				memset(svrRecvBuf, 0, SzBuf);
				
				/* 크리티컬 이슈 발견 및 임시해결! */
				/* 가끔, 메인 서버에서 306byte 이상으로 데이터를 몰아보낼 때가 있기 때문에, 일단은 306씩 읽도록 고정해야 한다.		*/
				/* 보다 안정적인 해결방안을 위해서는, 읽은 데이터를 임시보관할 장소를 만들고, 패킷을 '완성'시켜 내보내도록 구현해야 함.		*/
				/* 또한, 로그 버퍼의 크기가 512로 작성되어 있음을 기억하자!                         						*/
				
				/* [해결안] */
				/* 1. 4byte(길이부) 만큼 읽어본다																			*/
				/* 2. 4byte보다 짧으면 오류 반환, 4byte를 읽었으면 정수로 변환하여 읽어야 할 길이로 사용한다								*/
				/* 3. 읽어야 할 길이만큼 FD_SET등 일련의 과정을 반복하여 읽는다. 읽어야 할 길이 - 읽은 길이 하여 0이 되거나 타임아웃(0.1s) 될때까지 반복한다.	*/
				
				/* if (SOCKET_ERROR == (svrRecvLen = recv(tempSvrSocket, svrRecvBuf, 306, 0))) - Old version */
				if (SOCKET_ERROR == (svrRecvLen = recv(tempSvrSocket, svrRecvBuf, 4, 0)))
				{
					errCode = WSAGetLastError();
					
					if (errCode == WSAECONNRESET)
					{
						svrSocket[i] = 0;
						fprintf(stderr, "서버(%s:%d)와 연결이 비정상 종료됨. (Session:%d)\n", inet_ntoa(svrAddr.sin_addr), ntohs(svrAddr.sin_port), curSessionIdx);
						writeLog(g_LOG, NULL, 0, "rcvsvr(socket closed by error", ")");
						continue;
					}
				}
				else if (0 == svrRecvLen)
				{
					svrSocket[i] = 0;
					fprintf(stderr, "서버(%s:%d)에서 연결 종료시킴. (Session:%d)\n", inet_ntoa(svrAddr.sin_addr), ntohs(svrAddr.sin_port), curSessionIdx);
					writeLog(g_LOG, NULL, 0, "rcvsvr(socket closed by server", ")");
					continue;
				}
				else
				{
					int restLen, readLen;
					fd_set locFdSet;
					struct timeval loctv;
					long long locTimeout;
					
					/* 4byte 데이터 길이를 계산 */
					if (svrRecvLen != 4)
					{
						writeLog(g_LOG, NULL, 0, "rcvsvr(fail to read 4byte of header", ")");
						continue;
					}
					
					svrRecvBuf[svrRecvLen] = '\0';
					restLen = atoi(svrRecvBuf);
					readLen = 4;
					loctv.tv_sec = 0;
					loctv.tv_usec = 100000; /* 0.1초 */
					locTimeout = currentTimeMillis() + 100LL; /* 0.1초 */
					
					/* 셀렉터를 다시 등록, 남은 길이를 다 읽도록 함 */
					while (1)
					{
						if (restLen <= 0) break;
						
						FD_ZERO(&locFdSet);
						FD_SET(tempSvrSocket, &locFdSet);
						
						if (select(0, &locFdSet, NULL, NULL, &loctv) == 1 && FD_ISSET(tempSvrSocket, &locFdSet))
						{
							if (SOCKET_ERROR == (readLen = recv(tempSvrSocket, svrRecvBuf + readLen, restLen, 0)))
							{
								errCode = WSAGetLastError();
								
								if (errCode == WSAECONNRESET)
								{
									svrSocket[i] = 0;
									fprintf(stderr, "서버(%s:%d)와 연결이 비정상 종료됨. (Session:%d)\n", inet_ntoa(svrAddr.sin_addr), ntohs(svrAddr.sin_port), curSessionIdx);
									writeLog(g_LOG, NULL, 0, "rcvsvr(socket closed by error", ")");
									break;
								}
							}
							else if (0 == readLen)
							{
								svrSocket[i] = 0;
								fprintf(stderr, "서버(%s:%d)에서 연결 종료시킴. (Session:%d)\n", inet_ntoa(svrAddr.sin_addr), ntohs(svrAddr.sin_port), curSessionIdx);
								writeLog(g_LOG, NULL, 0, "rcvsvr(socket closed by server", ")");
								break;
							}
							else
							{
								restLen -= readLen;
								svrRecvLen += readLen;
							}
						}
						
						if (currentTimeMillis() > locTimeout) break;
					}
					
					if (restLen != 0) continue; /* 0.1초를 더 주었음에도 다 읽지 못한 경우이거나 더 읽은 경우는 전송 포기 */
					
					/* Svr recv logging */
					writeLog(g_LOG, svrRecvBuf, svrRecvLen, "rcvsvr(", ")");
					
					/* Send to client */
					memset(aryBuf, 0x00, sizeof(aryBuf));	/* 예비에서 msgId 추출 */
					memcpy(aryBuf, svrRecvBuf + 285, 10);
					memset(svrRecvBuf + 285, ' ', 10);		/* 예비 공백문자 복원 */
					idx = atoi(aryBuf);
					tempCliSocket = 0;
					
					for (j = 0; j < MaxClients; ++j)
					{
						if (cliSocket[j].msgId == idx)
						{
							tempCliSocket = cliSocket[j].hSoc;
							break;
						}
					}
					
					if (tempCliSocket == 0) continue; /* 목표 클라이언트 소켓이 사라진 상태, 전송 포기 */
					
					send(tempCliSocket, svrRecvBuf, svrRecvLen, 0);
					
					/* Remove client */
					closesocket(tempCliSocket);
					clrCliSocketByIdx(j, cliSocket);
					
					fprintf(stderr, "클라이언트(%s:%d:%d) 접속 종료시킴.\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), idx);
					
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
		if (cliSocket[i].hSoc != 0)
		{
			closesocket(cliSocket[i].hSoc);
			cliSocket[i].hSoc = 0;
			cliSocket[i].dwConTime = 0;
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

int clrCliSocket(SOCKET hSocket, TOSocket *pSocket, int length) {
	int i = 0;
	
	for (i = 0; i < length; ++i)
	{
		if (pSocket[i].hSoc == hSocket)
		{
			memset(&pSocket[i], 0x00, sizeof(TOSocket));
			return 0;
		}
	}
	
	return -1;
}

int clrCliSocketByIdx(int index, TOSocket *pSocket)
{
	memset(&pSocket[index], 0x00, sizeof(TOSocket));
	return 0;
}