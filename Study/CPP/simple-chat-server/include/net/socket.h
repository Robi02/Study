#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "common.h"
#include <sys/socket.h>

namespace robi_net {

class Socket
{
public:
    enum ConnectionType
    {
        UNKNOWN = -1, TCP, UDP, MAX
    };

protected:
    int fd_; // socket file discriptor

public:
    inline int get_fd() { return this->fd_; }
    inline int set_fd(int fd) { this->fd_ = fd; }

    Socket();
    virtual ~Socket();

    bool IsValid();
    int Create(ConnectionType connection_type);
    int SetSockOpt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
    int Bind(int svr_ip, int port);
    int Listen(int backlog);
    int Accept(struct sockaddr_in *out_cli_addr);
    int Connect(const char *svr_ip, int port);
};}

#endif