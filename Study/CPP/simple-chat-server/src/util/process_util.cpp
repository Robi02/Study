#include "process_util.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>

namespace robi_util
{

int MakeDaemon()
{
    pid_t pid;

	if ((pid = fork()) < 0)
	{
		fprintf(stderr, "Parent's fork() FAILED! (%s)\n", strerror(errno));
		return -1;
	}
	else if (pid != 0)
	{
		exit(0); // parent process exit
	}

	if (setsid() < 0)
	{
		fprintf(stderr, "Child's setsid() FAILED! (%s)\n", strerror(errno));
		return -1;
	}

    return 0;
}

}