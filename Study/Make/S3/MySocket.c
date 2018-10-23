#include <WinSock2.h>
#include "MyThread.h"
#include "MyMutex.h"
#include "MySemaphore.h"
#include "MySocket.h"

int MySocket()
{
    WSADATA stWsaData;
	
	if (WSAStartup(MAKEWORD(2, 2), &stWsaData) != 0)
    {
        return -1;        
    }

    MyThread();
    MyMutex();
    MySemaphore();

    if (WSACleanup() != 0)
    {
        return -1;
    }

    return 0;
}