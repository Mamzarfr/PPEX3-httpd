#define _XOPEN_SOURCE 500

#include "http.h"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 4096
#define BUF_SIZE_SMALL 128

static void send_response(int fd, const char *data, ssize_t len)
{
    ssize_t total = 0;
    ssize_t sent = 0;

    while (total < len)
    {
        sent = write(fd, data + total, len - total);
        if (sent == -1)
        {
            fprintf(stderr, "Send error\n");
            return;
        }
        total += sent;
    }
}

static void free_request(struct req *req)
{
    if (!req)
        return;

    free(req->method);
    free(req->path);
    free(req->version);
    free(req->host);
    free(req);
}

static char *find_endl(char *str)
{
    char *endl = strstr(str, "\r\n");
    return endl;
}

static int parse_l(char *line, struct req *req)
{
    char *method = strtok(line, " ");
    char *path = strtok(NULL, " ");
    char *ver = strtok(NULL, "\r");

    if (!method || !path || !ver)
        return -1;

    req->method = strdup(method);
    req->path = strdup(path);
    req->version = strdup(ver);

    if (!req->method || !req->path || !req->version)
        return -1;

    return 0;
}

static int parse_req(char *buffer, struct req *req)
{
    char *endl = find_endl(buffer);
    if (!endl)
        return -1;

    *endl = '\0';

    if (parse_l(buffer, req) != 0)
        return -1;

    char *head = endl + 2;

    endl = find_endl(head);
    while (endl != NULL)
    {
        if (endl == head)
            break;

        *endl = '\0';

        if (strncmp(head, "Host:", 5) == 0)
        {
            char *host = head + 5;
            while (*host == ' ')
                host++;
            req->host = strdup(host);
        }

        head = endl + 2;
        endl = find_endl(head);
    }

    return 0;
}

static void get_http_date(char *buf)
{
    time_t t = time(NULL);
    struct tm *gmt = gmtime(&t);
    strftime(buf, BUF_SIZE_SMALL, "%a, %d %b %Y %H:%M:%S GMT", gmt);
}

static void send_full(int fd, const char *status, const char *body,
                      ssize_t body_len)
{
    char date[BUF_SIZE_SMALL];
    get_http_date(date);

    char header[BUF_SIZE];
    int n = snprintf(header, BUF_SIZE,
                     "HTTP/1.1 %s\r\n"
                     "Date: %s\r\n"
                     "Content-Length: %zd\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     status, date, body_len);

    if (n > 0)
    {
        send_response(fd, header, n);
        if (body && body_len > 0)
            send_response(fd, body, body_len);
    }
}

static char *read_file(const char *path, ssize_t *out_size)
{
    int fd = open(path, O_RDONLY);
    if (fd == -1)
        return NULL;

    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        close(fd);
        return NULL;
    }

    ssize_t size = st.st_size;
    char *buf = malloc(size);
    if (!buf)
    {
        close(fd);
        return NULL;
    }

    ssize_t total = 0;
    ssize_t n;
    while (total < size)
    {
        n = read(fd, buf + total, size - total);
        if (n <= 0)
        {
            free(buf);
            close(fd);
            return NULL;
        }
        total += n;
    }

    close(fd);
    *out_size = size;
    return buf;
}

void http_handle_request(int client_fd, struct config *cfg)
{
    char buffer[BUF_SIZE];
    ssize_t bytes = recv(client_fd, buffer, BUF_SIZE - 1, 0);

    if (bytes <= 0)
        return;

    buffer[bytes] = '\0';

    struct req *req = calloc(1, sizeof(struct req));
    if (!req)
        return;

    if (parse_req(buffer, req) != 0)
    {
        send_full(client_fd, "400 Bad Request", NULL, 0);
        free_request(req);
        return;
    }

    if (!req->host)
    {
        send_full(client_fd, "400 Bad Request", NULL, 0);
        free_request(req);
        return;
    }

    if (strcmp(req->version, "HTTP/1.1") != 0)
    {
        send_full(client_fd, "505 HTTP Version Not Supported", NULL, 0);
        free_request(req);
        return;
    }

    if (strcmp(req->method, "GET") != 0 && strcmp(req->method, "HEAD") != 0)
    {
        send_full(client_fd, "405 Method Not Allowed", NULL, 0);
        free_request(req);
        return;
    }

    char path[BUF_SIZE_SMALL];
    size_t len = strlen(req->path);

    if (len > 0 && req->path[len - 1] == '/')
    {
        snprintf(path, BUF_SIZE_SMALL, "%s%s%s", cfg->servers->root_dir,
                 req->path, cfg->servers->default_file);
    }
    else
    {
        snprintf(path, BUF_SIZE_SMALL, "%s%s", cfg->servers->root_dir,
                 req->path);
    }

    ssize_t size;
    char *body = read_file(path, &size);

    if (!body)
    {
        if (errno == EACCES)
            send_full(client_fd, "403 Forbidden", NULL, 0);
        else
            send_full(client_fd, "404 Not Found", NULL, 0);
    }
    else
    {
        if (strcmp(req->method, "HEAD") == 0)
            send_full(client_fd, "200 OK", NULL, size);
        else
            send_full(client_fd, "200 OK", body, size);
        free(body);
    }

    free_request(req);
}
