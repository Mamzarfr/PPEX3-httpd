#ifndef SERVER_H
#define SERVER_H

#include "../config/config.h"

int server_create_and_bind(const char *ip, const char *port);
void server_start(int server_socket, struct config *cfg);

#endif // SERVER_H
