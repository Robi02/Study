#ifndef __SOCKET_UTIL_H__
#define __SOCKET_UTIL_H__

namespace robi_util
{
    int CreateServerSocket(int protocol, long in_addr, short in_port);
    int InitEpoll(int svr_socket_fd, int max_connections);
    int ReadSocketFromEpoll(int epoll_fd);
}

#endif