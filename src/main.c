#define _POSIX_C_SOURCE 200809L

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "config/config.h"
#include "daemon/daemon.h"
#include "logger/logger.h"
#include "server/server.h"

int main(int argc, char **argv)
{
    struct config *cfg = parse_configuration(argc, argv);
    if (!cfg)
        return 2;

    if (cfg->daemon == START)
    {
        if (daemon_start(cfg->pid_file) == -1)
        {
            config_destroy(cfg);
            errx(1, "daemon start error");
        }
    }
    else if (cfg->daemon == STOP)
    {
        if (daemon_stop(cfg->pid_file) == -1)
        {
            config_destroy(cfg);
            errx(1, "daemon stop error");
        }
        config_destroy(cfg);
        return 0;
    }
    else if (cfg->daemon == RESTART)
    {
        daemon_stop(cfg->pid_file);
        if (daemon_start(cfg->pid_file) == -1)
        {
            config_destroy(cfg);
            errx(1, "daemon rest error");
        }
    }

    int fd = server_create_and_bind(cfg->servers->ip, cfg->servers->port);
    if (fd == -1)
    {
        config_destroy(cfg);
        errx(1, "create and bind error");
    }

    if (cfg->log)
    {
        if (logger_init(cfg->log_file) != 0)
        {
            close(fd);
            config_destroy(cfg);
            errx(1, "init logger err");
        }
    }

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, NULL);

    server_start(fd, cfg);

    logger_close();
    close(fd);
    config_destroy(cfg);
    return 0;
}
