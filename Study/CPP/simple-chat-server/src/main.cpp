#include "common.h"
#include "util/time_util.h"
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <signal.h>
#include <time.h>

static const int kMaxConnections = 5000;
static const int kSvrPort = 45312;
static const int kEpollWaitTimeout = -1;

void Sigterm(int sig_type)
{
	fprintf(stdout, "SIGTERM(%d) detected but, signal will be ignored.\n", sig_type);
	return;
}

int main(int argc, char **argv)
{
	int pid = -1;
	int svr_socket_fd = -1;
	int epoll_fd = -1;

	char time_buf[32];
	const char *time_fmt = "[%F %T]"; // [yyyy-MM-dd HH:mm:ss]

	robi_util::TimeUtil::GetSimpleDateFormat(time_buf, sizeof(time_buf), time_fmt, robi_util::TimeUtil::GetCurrentTimeMillis());
	Logger::Info("%s", time_buf);
	return 0;

	/*
	// Server daemonizing
	if (robi_util::MakeDaemon() < 0)
	{
		fprintf(stderr, "MakeDaemon() FAILED!\n");
		exit(-1);
	}

	// Server process setting
	signal(SIGTERM, Sigterm);
	fprintf(stdout, "Daemon process running... (pid:%d)\n", pid);

	// Server socket setting
	if ((svr_socket_fd = robi_util::CreateServerSocket(IPPROTO_TCP, INADDR_ANY, kSvrPort)) < 0)
	{
		fprintf(stderr, "CreateServerSocket() FAILED!\n");
		exit(-1);
	}

	// Server epoll setting
	if ((epoll_fd = robi_util::InitEpoll(svr_socket_fd, kMaxConnections)) < 0)
	{
		fprintf(stderr, "InitEpoll() FAILED!\n");
		exit(-1);
	}

	// Server main loop
	struct epoll_event epoll_event_ary[kMaxConnections];
	int eventCnt = -1;

	while (true)
	{
		if ((eventCnt = epoll_wait(epoll_fd, epoll_event_ary, kMaxConnections, kEpollWaitTimeout)) < 0)
		{
			fprintf(stderr, "'eventCnt' < 0 ! (%s)\n", strerror(errno));
			break;
		}

		for (int i = 0; i < eventCnt; ++i)
		{
			if (epoll_event_ary[i].data.fd == svr_socket_fd) // accept client
			{
				struct sockaddr_in cli_socket_addr;
				socklen_t cli_addr_len = sizeof(cli_socket_addr);
				int cli_socket_fd = accept(svr_socket_fd, (struct sockaddr *)&cli_socket_addr, &cli_addr_len);

				if (cli_socket_fd < 0)
				{
					fprintf(stderr, "'cli_socket_fd' < 0 !\n");
					continue;
				}

				write(cli_socket_fd, "OK", 3);

				struct epoll_event epoll_default_event;

				epoll_default_event.events = EPOLLIN;
				epoll_default_event.data.fd = cli_socket_fd;
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cli_socket_fd, &epoll_default_event);
			}
			else // recv client data
			{
				int cli_socket_fd = epoll_event_ary[i].data.fd;
				char buf[256];

				memset(buf, 0x00, 256);
				buf[0] = '\n';
				
				read(cli_socket_fd, (buf + 1), 255);
				write(cli_socket_fd, buf, strlen(buf) + 1);
				close(cli_socket_fd);
			}
		}

		sleep(1);
	}	

	// Server deinitializing
	close(svr_socket_fd);
	close(epoll_fd);
	return 0;
	*/
}
