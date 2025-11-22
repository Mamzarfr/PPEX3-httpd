#include "config.h"

#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/string/string.h"

static enum daemon parse_dopts(const char *value)
{
    if (!value)
        return NO_OPTION;
    if (strcmp(value, "start") == 0)
        return START;
    if (strcmp(value, "stop") == 0)
        return STOP;
    if (strcmp(value, "restart") == 0)
        return RESTART;
    return NO_OPTION;
}

static bool parse_lopts(const char *value)
{
    if (!value)
        return true;
    return strcmp(value, "true") == 0;
}

static struct option *get_lopts(void)
{
    static struct option lopts[] = {
        { "pid_file", required_argument, 0, 'p' },
        { "log_file", required_argument, 0, 'l' },
        { "log", required_argument, 0, 'L' },
        { "server_name", required_argument, 0, 's' },
        { "port", required_argument, 0, 'P' },
        { "ip", required_argument, 0, 'i' },
        { "root_dir", required_argument, 0, 'r' },
        { "default_file", required_argument, 0, 'd' },
        { "daemon", required_argument, 0, 'D' },
        { 0, 0, 0, 0 }
    };
    return lopts;
}

static int handle_opt(int opt, struct config *cfg, struct server_config *scfg)
{
    switch (opt)
    {
    case 'p':
        cfg->pid_file = strdup(optarg);
        break;
    case 'l':
        cfg->log_file = strdup(optarg);
        break;
    case 'L':
        cfg->log = parse_lopts(optarg);
        break;
    case 's':
        scfg->server_name = string_create(optarg, strlen(optarg));
        if (!scfg->server_name)
            return -1;
        break;
    case 'P':
        scfg->port = strdup(optarg);
        break;
    case 'i':
        scfg->ip = strdup(optarg);
        break;
    case 'r':
        scfg->root_dir = strdup(optarg);
        break;
    case 'd':
        scfg->default_file = strdup(optarg);
        break;
    case 'D':
        cfg->daemon = parse_dopts(optarg);
        break;
    default:
        return -1;
    }
    return 0;
}

static int valid(struct config *cfg, struct server_config *scfg)
{
    if (!scfg->default_file)
        scfg->default_file = strdup("index.html");

    if (cfg->daemon != NO_OPTION && !cfg->log_file)
        cfg->log_file = strdup("HTTPd.log");

    if (!cfg->pid_file || !scfg->server_name || !scfg->port || !scfg->ip
        || !scfg->root_dir || !scfg->default_file)
        return -1;

    return 0;
}

struct config *parse_configuration(int argc, char *argv[])
{
    struct config *cfg = calloc(1, sizeof(struct config));
    if (!cfg)
        return NULL;

    cfg->log = true;
    cfg->daemon = NO_OPTION;

    struct server_config *scfg = calloc(1, sizeof(struct server_config));
    if (!scfg)
    {
        free(cfg);
        return NULL;
    }
    cfg->servers = scfg;

    int opt;
    int i = 0;
    struct option *lopts = get_lopts();

    while ((opt = getopt_long(argc, argv, "", lopts, &i)) != -1)
    {
        if (handle_opt(opt, cfg, scfg) != 0)
        {
            config_destroy(cfg);
            return NULL;
        }
    }

    if (valid(cfg, scfg) == -1)
    {
        config_destroy(cfg);
        return NULL;
    }

    return cfg;
}

void config_destroy(struct config *cfg)
{
    if (!cfg)
        return;

    free(cfg->pid_file);
    free(cfg->log_file);

    if (cfg->servers)
    {
        string_destroy(cfg->servers->server_name);
        free(cfg->servers->port);
        free(cfg->servers->ip);
        free(cfg->servers->root_dir);
        free(cfg->servers->default_file);
        free(cfg->servers);
    }

    free(cfg);
}
