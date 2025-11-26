#define _POSIX_C_SOURCE 200809L

#include "daemon.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int daemon_start(const char *pid_file)
{
    pid_t pid = fork();
    if (pid == -1)
        return -1;

    if (pid > 0)
        exit(0);

    if (setsid() == -1)
        return -1;

    pid = fork();

    if (pid > 0)
        exit(0);

    int fd = open("/dev/null", O_RDWR);
    if (fd != -1)
    {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > 2)
            close(fd);
    }

    FILE *f = fopen(pid_file, "w");
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
    if (!f)
        return -1;

    pid_t pid;
    if (fscanf(f, "%d", &pid) != 1)
    {
        fclose(f);
        return -1;
    }
    fclose(f);

    if (kill(pid, SIGTERM) == -1)
        return -1;

    remove(pid_file);
    return 0;
}
