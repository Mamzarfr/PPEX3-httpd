#define _POSIX_C_SOURCE 200809L

#include "logger.h"

#include <stdio.h>
#include <time.h>

#include "../http/http.h"

static FILE *log_file = NULL;

static void get_date(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm *gmt = gmtime(&now);
    strftime(buffer, size, "%a, %d %b %Y %H:%M:%S GMT", gmt);
}

int logger_init(const char *log)
{
    if (log)
    {
        log_file = fopen(log, "a");
        if (!log_file)
            return -1;
    }
    else
    {
        log_file = stdout;
    }
    return 0;
}

void log_req(const char *server_name, const char *method, const char *target,
             const char *ip)
{
    if (!log_file)
        return;

    char date[128];
    get_date(date, sizeof(date));

    if (method && target)
    {
        fprintf(log_file, "%s [%s] received %s on '%s' from %s\n", date,
                server_name, method, target, ip);
    }
    else
    {
        fprintf(log_file, "%s [%s] received Bad Request from %s\n", date,
                server_name, ip);
    }
    fflush(log_file);
}

void log_resp(const char *server_name, int status, const char *ip,
              const struct req *req)
{
    if (!log_file)
        return;

    char date[128];
    get_date(date, sizeof(date));

    if (req && req->method && req->path)
    {
        fprintf(log_file, "%s [%s] responding with %d to %s for %s on '%s'\n",
                date, server_name, status, ip, req->method, req->path);
    }
    else if (req && !req->method && req->path)
    {
        fprintf(log_file,
                "%s [%s] responding with %d to %s for UNKNOWN on '%s'\n", date,
                server_name, status, ip, req->path);
    }
    else
    {
        fprintf(log_file, "%s [%s] responding with %d to %s\n", date,
                server_name, status, ip);
    }
    fflush(log_file);
}

void logger_close(void)
{
    if (log_file && log_file != stdout)
    {
        fclose(log_file);
        log_file = NULL;
    }
}
