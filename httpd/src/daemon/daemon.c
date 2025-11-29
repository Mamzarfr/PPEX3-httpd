#define _POSIX_C_SOURCE 200809L

#include "daemon.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int daemon_start(const char *pid_file)
{
    FILE *f = fopen(pid_file, "r");
    if (f)
    {
        pid_t tmppid;
        if (fscanf(f, "%d", &tmppid) == 1)
        {
            if (kill(tmppid, 0) == 0)
            {
                fclose(f);
                return -1;
            }
        }
        fclose(f);
    }

    pid_t pid = fork();
    if (pid == -1)
        return -1;

    if (pid > 0)
        exit(0);

    f = fopen(pid_file, "w");
    if (f)
    {
        fprintf(f, "%d\n", getpid());
        fclose(f);
    }

    return 0;
}

int daemon_stop(const char *pid_file)
{
    FILE *f = fopen(pid_file, "r");
    if (f)
    {
        pid_t pid;
        if (fscanf(f, "%d", &pid) == 1)
        {
            kill(pid, SIGINT);
        }
        fclose(f);
    }

    remove(pid_file);
    return 0;
}
