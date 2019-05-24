#include "socket_util.h"
#include <errno.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

namespace robi_util {

static const int kListenLogback = 256;

int CreateServerSocket(int protocol, long in_addr, short in_port)
{
    const int socket_domain   = AF_INET;
    const int socket_type     = (protocol == IPPROTO_TCP ? SOCK_STREAM : SOCK_DGRAM);
    const int socket_protocol = protocol;

    // [SocketType]
    // SOCK_DGRAM  : UDP
    // SOCK_RAW    : IP/ICMP
    // SOCK_STREAM : TCP

    int socket_fd = socket(socket_domain, socket_type, socket_protocol);

	if (socket_fd < 0)                                                              
	{
		fprintf(stderr, "FAIL to create server socket! (%s)\n", strerror(errno));
        exit(-1);
	}

	struct sockaddr_in soc_addr; 

	memset(&soc_addr, 0x00, sizeof(struct sockaddr_in));
	soc_addr.sin_family = AF_INET;
	soc_addr.sin_addr.s_addr = in_addr;
	soc_addr.sin_port = htons(in_port);

	if (bind(socket_fd, (struct sockaddr *)&soc_addr, sizeof(soc_addr)) < 0)
	{
		fprintf(stderr, "bind() FAILED! (%s)\n", strerror(errno));
		return -1;
	}

	if (listen(socket_fd, kListenLogback) < 0)
	{
		fprintf(stderr, "listen() FAILED! (%s)\n", strerror(errno));
		return -1;
	}

    return socket_fd;
}

int InitEpoll(int svr_socket_fd, int max_connections)
{
    if (svr_socket_fd < 0)
    {
        fprintf(stderr, "'svr_socket_fd' < 0 !\n");
        return -1;
    }

    if (max_connections < 0)
    {
        fprintf(stderr, "'max_connections' < 0 !\n");
        return -1;
    }

    int epoll_fd = epoll_create(max_connections);

    if (epoll_fd < 0)
    {
        fprintf(stderr, "epoll_create() FAILED! (%s)\n", strerror(errno));
        return -1;
    }

    // [sys/epoll.h]
    // typedef union epoll_data     struct epoll_event
    // {                            {
    //   void *ptr;                     uint32_t events     // Epoll events
    //   int fd;                        epoll_data_t data;  // User data variable
    //   uint32_t u32;              };
    //   uint64_t u64;              
    // } epoll_data_t;              

    struct epoll_event epoll_event_default;

    memset(&epoll_event_default, 0x00, sizeof(epoll_event_default));
	epoll_event_default.events = EPOLLIN;
	epoll_event_default.data.fd = svr_socket_fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, svr_socket_fd, &epoll_event_default);
    return epoll_fd;
}


void test()
{
    
}

}