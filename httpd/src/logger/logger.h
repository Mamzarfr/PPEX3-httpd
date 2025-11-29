#ifndef LOGGER_H
#define LOGGER_H

#include <stddef.h>

struct req;

int logger_init(const char *log_file);
void log_req(const char *server_name, const char *method, const char *target,
             const char *ip);
void log_resp(const char *server_name, int status, const char *ip,
              const struct req *req);
void logger_close(void);

#endif // LOGGER_H
