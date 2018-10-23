#include "stdheader.h"

int main(int argc, char** argv);
int checkArgc(int argc);
int clientWork();

/*
 *
 * 1. main.c : 프로그램 시작 및 메인 로직
 * 2. commonlib.c : 자주 사용하는 공통함수
 * 3. global.c : 프로그램 전역에서 사용되는 전역변수들
 * 4. msgfileio.c : 전문 및 로그 입출력
 * 5. msgsocket.c : 전문 패킷화 및 복원, 서버로 송수신
 * 6. record.c : 전문 읽기 및 변환, 출력 등
 *
 */