#ifndef __SVRSOCKET_H__
#define __SVRSOCKET_H__

#include "stdheader.h"

int connectSocket(SOCKET *pSocket, char *pIP, int port);
int closeSocket(SOCKET *pSocket);

#endif