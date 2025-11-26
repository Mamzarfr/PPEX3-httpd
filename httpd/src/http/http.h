#ifndef HTTP_H
#define HTTP_H

#include "../config/config.h"

struct req
{
    char *method;
    char *path;
    char *version;
    char *host;
};

void http_handle_request(int client_fd, struct config *cfg);

#endif // HTTP_H
