#define _POSIX_C_SOURCE 200112L

#include "server.h"

#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../http/http.h"

int server_create_and_bind(const char *ip, const char *port)
{
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *res = NULL;

    if (getaddrinfo(ip, port, &hints, &res) == -1)
    {
        fprintf(stderr, "create_and_bind: failed getaddrinfo\n");
        return -1;
    }

    int sockfd = -1;

    for (struct addrinfo *p = res; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
            continue;

        int yes = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) != -1)
            break;

        close(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(res);
    return sockfd;
}

void server_start(int server_socket, struct config *cfg)
{
    if (listen(server_socket, SOMAXCONN) == -1)
        return;

    while (1)
    {
        int cfd = accept(server_socket, NULL, NULL);
        if (cfd != -1)
        {
            fprintf(stderr, "Client connected\n");
            http_handle_request(cfd, cfg);
            close(cfd);
            fprintf(stderr, "Client disconnected\n");
        }
    }
}
