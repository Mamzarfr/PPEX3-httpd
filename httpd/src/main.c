#include <err.h>
#include <stdio.h>
#include <unistd.h>

#include "config/config.h"
#include "daemon/daemon.h"
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
            errx(1, "Failed to daemonize");
        }
    }
    else if (cfg->daemon == STOP)
    {
        if (daemon_stop(cfg->pid_file) == -1)
        {
            config_destroy(cfg);
            errx(1, "Failed to stop daemon");
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
            errx(1, "Failed to restart daemon");
        }
    }

    int fd = server_create_and_bind(cfg->servers->ip, cfg->servers->port);
    if (fd == -1)
    {
        config_destroy(cfg);
        errx(1, "Failed to create and bind socket");
    }

    fprintf(stderr, "Server listening on %s:%s\n",
            cfg->servers->ip, cfg->servers->port);

    server_start(fd, cfg);

    close(fd);
    config_destroy(cfg);
    return 0;
}
