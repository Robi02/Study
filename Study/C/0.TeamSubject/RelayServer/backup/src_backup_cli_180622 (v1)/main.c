#include "main.h"
#include "commonlib.h"
#include "msgfileio.h"
#include "record.h"
#include "msgsocket.h"
#include "global.h"

/*
 * [ Workflow ]
 * 1. 전문파일 열기
 * 2. 한 라인 읽기			   -- (라인 개수만큼 반복)
 * 3. 레코드 분류				|
 * 4. 서버전문으로 변환			|
 * 5. 실패값으로 파일에 쓰기		| [개선]
 * 6. 서버로 전송하기(일회용소캣)		| -> 12. 재사용 소켓으로 [0000:]와 같이 헤더 데이터를 추가하여 전송
 * 7. 서버에서 응답받기			| -> 13. 연결이 끊어졌을 시 재연결하여 전송
 * 8. 신전문으로 변경하기			|
 * 9. 변경된 전문 쓰기		   --
 * 10. 표제부, 종료부 쓰기
 * 11. 모든 전문 파일 닫기
 * +1. 로그 파일 작성
 * +2. 전문 콘솔로 출력
 * +3. 환경값 global.c에서 관리
 *
 * < 1차 과제 >
 * (1), (2), (3), (+2)
 *
 * < 2차 과제 >
 * [1], [2], [3], (4), (5), (6), (7), (8), (9), (10), (11), [+2], (+3)
 *
 */

int main(int argc, char **argv) {
	if (checkArgc(argc) != 0)
	{
		return -1;
	}
	
	if (initGlobals(argc, argv) != 0)
	{
		return -1;
	}

	if (clientWork() != 0)
	{
		return -1;
	}
	
	return 0;
}

int checkArgc(int argc) {
	if (argc <= 1)
	{
		fprintf(stderr, "'startCli.bat' 파일로 프로그램을 시작하십시오.\n");
		return -1;
	}
	
	return 0;
}

int clientWork() {
	char arLineBuf[512] = { 0, };
	int szLineBuf = sizeof(arLineBuf);
	CliRecord cliRecord;
	SvrRecord svrRecord;
	long startTime = 0;
	int szCliRecord = sizeof(CliRecord);
	int szSvrRecord = sizeof(SvrRecord);
	FILE* pInMsgFile = NULL;
	FILE* pOutMsgFile = NULL;
	int szReadMsg = -1;
	int lineIdx = 0;
	
	fprintf(stdout, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	fprintf(stdout, "* 프로그램 시작...\n");
	startTime = currentTimeMillis();

	/* 전문파일 I/O */
	pInMsgFile = openMsgFile(g_IN_MSG_FILE_PATH, "r+");		/* 입력 전문파일 열기(읽기전용) */
	pOutMsgFile = openMsgFile(g_OUT_MSG_FILE_PATH, "w");	/* 출력 전문파일 생성(쓰기전용) */
	g_LOG = openMsgFile(g_OUT_LOG_FILE_PATH, "w");			/* 출력 전문파일 생성(쓰기전용) */
	
	if (pInMsgFile == NULL || pOutMsgFile == NULL || g_LOG == NULL)
	{
		fprintf(stdout, "* 전문/로그 열기 또는 생성 실패!\n");
		goto exit;
	}
	
	fprintf(stdout, "* 전문/로그 열기 및 생성 완료!\n");
	
	/* 전환된 표제부 파일에 쓰기 */
	initRecord(&cliRecord, szCliRecord);
	initRecord(&svrRecord, szSvrRecord);
	cvtSvr2CliRecord(&svrRecord, &cliRecord, 0);
	writeMsg(pOutMsgFile, byteToString(arLineBuf, (char*)&cliRecord.stHead, sizeof(CliHeadRecord)), 0, 0);	
	fprintf(stdout, "* 표제부 작성 완료!\n");
	fprintf(stdout, "* 전문 읽기 및 송수신중...\n");
	
	while (1)
	{
		++lineIdx;
		initRecord(&cliRecord, szCliRecord);
		initRecord(&svrRecord, szSvrRecord);
		
		/* 한 라인 읽기 */
		if ((szReadMsg = readMsgLine(pInMsgFile, arLineBuf, szLineBuf)) <= 0)
		{
			break;
		}
		
		/* 레코드 분류및 생성 */		
		switch (arLineBuf[0])
		{
			case 'D':
			{
				if (makeRecord(arLineBuf, szReadMsg, &(cliRecord.stData), sizeof(CliDataRecord)) < 0)
				{
					fprintf(stderr, "클라 데이터부(Idx:%d) 생성 실패.\n", lineIdx);
				}
				
				break;
			}
			case 'S':
			case 'E':
			{
				--lineIdx;
				continue;
			}
			default:
			{
				fprintf(stderr, "미정의 데이터. (line:%d, idCode:%c).\n", lineIdx, arLineBuf[0]);
				continue;
			}
		}
		
		/* 서버전문으로 변환 */
		if (cvtCli2SvrRecord(&cliRecord, &svrRecord) < 0)
		{
			continue;
		}
		
		/* 원본값으로 출력 파일에 쓰기 */
		writeMsg(pOutMsgFile, byteToString(arLineBuf, (char*)&cliRecord.stData, sizeof(CliDataRecord)), lineIdx, 0);
		
		/* 서버로 송수신 및 로깅(일회용 소캣) */
		if (sendAndRecv(&svrRecord, 0) == -1)
		{
			continue;
		}
		
		/* 클라전문으로 변환 */
		if (cvtSvr2CliRecord(&svrRecord, &cliRecord, -1) < 0)
		{
			continue;
		}
		
		/* 결과값으로 출력 파일에 쓰기 */
		writeMsg(pOutMsgFile, byteToString(arLineBuf, (char*)&cliRecord.stData, sizeof(CliDataRecord)), lineIdx, 0);
	}
	
	fprintf(stdout, "* 전문 읽기 및 송수신, 파일 작성 완료!\n");

	/* 전환된 종료부 생성 및 파일에 쓰기 */
	initRecord(&cliRecord, szCliRecord);
	initRecord(&svrRecord, szSvrRecord);
	cvtSvr2CliRecord(&svrRecord, &cliRecord, 1);
	writeMsg(pOutMsgFile, byteToString(arLineBuf, (char*)&cliRecord.stTail, sizeof(CliTailRecord)), lineIdx, 0);
	fprintf(stdout, "* 종료부 작성 완료!\n");

exit:
	/* 모든 전문 파일 닫기 */
	closeMsgFile(pInMsgFile);
	closeMsgFile(pOutMsgFile);
	closeMsgFile(g_LOG);
	pInMsgFile = NULL;
	pOutMsgFile = NULL;
	g_LOG = NULL;
	
	fprintf(stdout, "* 프로그램 종료... (수행 시간: %.03f초)\n", timeDeltaMillis(startTime, currentTimeMillis()));
	fprintf(stdout, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	
	return 0;
}