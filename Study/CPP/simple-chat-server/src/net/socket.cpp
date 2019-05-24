#include "net/socket.h"
#include "util/logger.h"
#include <arpa/inet.h>
#include <netinet/in.h>

namespace robi_net {

Socket::Socket()
{
    this->fd_ = -1;
}

Socket::~Socket()
{
}

bool Socket::IsValid()
{
    if (this->fd_ < 0)
    {
        return false;
    }

    return true;
}

int Socket::Create(ConnectionType connection_type)
{
    const int domain    = AF_INET;
    const int type      = (connection_type == ConnectionType::TCP ? SOCK_STREAM : SOCK_DGRAM);
    const int protocol  = 0;
    
    int socket_fd = socket(domain, type, protocol);

    if (socket_fd < 0)
    {
        Logger::Error("socket() FAILED! (%s)", strerror(errno));
        return -1;
    }

    return (this->fd_ = socket_fd);
}

int Socket::SetSockOpt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    return -1;
}

int Socket::Bind(int svr_ip, int port)
{
    if (port < 0 || port > 65535)
    {
        Logger::Error("'port' range error! (port:%d)", port);
        return -1;
    }

    if (IsValid() == false)
    {
        Logger::Error("Socket NOT valid!");
        return -1;
    }

    struct ::sockaddr_in svr_socket_addr;
    socklen_t addr_len = sizeof(struct ::sockaddr_in);

    memset(&svr_socket_addr, 0x00, addr_len);
    svr_socket_addr.sin_family      = AF_INET;
    svr_socket_addr.sin_addr.s_addr = svr_ip;
    svr_socket_addr.sin_port        = ::htons(port);

    if (bind(this->fd_, (const struct ::sockaddr *)&svr_socket_addr, addr_len) < 0)
    {
        Logger::Error("bind() error! (%s)", strerror(errno));
        return -1;
    }

    return 0;
}

int Socket::Listen(int backlog)
{
    if (IsValid() == false)
    {
        Logger::Error("Socket NOT valid!");
        return -1;
    }

    if (backlog < 1)
    {
        Logger::Error("'backlog' less then 1!");
        return -1;
    }

    if (listen(this->fd_, backlog) < 0)
    {
        Logger::Error("'listen() FAILED! (%s)", strerror(errno));
        return -1;
    }

    return 0;
}

int Socket::Accept(struct sockaddr_in *out_cli_addr)
{
    if (IsValid() == false)
    {
        Logger::Error("Socket NOT valid!");
        return -1;
    }

    int cli_socket_fd = -1;
    socklen_t socket_len = sizeof(struct ::sockaddr_in);

    if ((cli_socket_fd = accept(this->fd_, (struct ::sockaddr *)out_cli_addr, &socket_len)) < 0)
    {
        Logger::Error("'accept() FAILED! (%s)", strerror(errno));
        return -1;
    }

    return cli_socket_fd;
}

int Socket::Connect(const char *svr_ip, int port)
{
    if (svr_ip == nullptr)
    {
        Logger::Error("'svr_ip' is null!");
        return -1;
    }

    if (port < 0 || port > 65535)
    {
        Logger::Error("'port' range error! (port:%d)", port);
        return -1;
    }

    if (IsValid() == false)
    {
        Logger::Error("Socket NOT valid!");
        return -1;
    }

    struct ::sockaddr_in svr_socket_addr;
    socklen_t addr_len = sizeof(struct ::sockaddr_in);

    memset(&svr_socket_addr, 0x00, addr_len);
    svr_socket_addr.sin_family      = AF_INET;
    svr_socket_addr.sin_addr.s_addr = inet_addr(svr_ip);
    svr_socket_addr.sin_port        = htons(port);

    if (connect(this->fd_, (struct sockaddr *)&svr_socket_addr, addr_len) < 0)
    {
        Logger::Error("connect() FAILED! (%s)", strerror(errno));
        return -1;
    }

    return 0;
}}