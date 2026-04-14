#define _POSIX_C_SOURCE 200112L

#include "server.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../http/http.h"

static volatile int sigint = 0;

static void sigint_handler(int useless)
{
    if (useless)
        sigint = 1;
    else
        sigint = 1;
}

int server_create_and_bind(const char *ip, const char *port)
{
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *res = NULL;

    if (getaddrinfo(ip, port, &hints, &res) == -1)
    {
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
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    if (listen(server_socket, SOMAXCONN) == -1)
        return;

    while (!sigint)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int cfd =
            accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (cfd != -1)
        {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip,
                      sizeof(client_ip));

            http_handle_request(cfd, cfg, client_ip);
            close(cfd);
        }
    }
}
