#define _POSIX_C_SOURCE 200809L

#include "server.h"

#include <netdb.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "config/config.h"
#include "http/create_response.h"
#include "http/parse_request.h"
#include "logger/logger.h"

int create_and_bind(const char *node, const char *service)
{
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *res = NULL;
    if (getaddrinfo(node, service, &hints, &res) == -1)
        return -1;

    struct addrinfo *p;
    int sock = -1;

    for (p = res; p; p = p->ai_next)
    {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == -1)
            continue;

        int yes = 1;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO | SO_REUSEADDR, &yes,
                   sizeof(yes));

        if (bind(sock, p->ai_addr, p->ai_addrlen) != -1)
            break;

        close(sock);
        sock = -1;
    }

    freeaddrinfo(res);

    if (p == NULL)
        return -1;

    return sock;
}

void send_res(int client_sock, char *response)
{
    ssize_t nb_sent;
    size_t total_sent = 0;
    while ((nb_sent = send(client_sock, response, strlen(response) - total_sent,
                           MSG_NOSIGNAL))
           > 0)
    {
        total_sent += nb_sent;
    }

    close(client_sock);
}

void check_error(struct config *conf, struct request *req)
{
    if (conf->error == 1 || req->err == 1)
    {
        req->status_code = "400";
        req->reason_phrase = "Bad Request";
    }
    if (req->err == 2)
    {
        req->status_code = "203";
        req->reason_phrase = "Forbidden";
    }
    if (req->err == 3)
    {
        req->status_code = "404";
        req->reason_phrase = "Not Found";
    }
}

static void check_req(struct config *conf, struct request *req)
{
    size_t size =
        strlen(conf->servers[0].ip) + strlen(conf->servers[0].port) + 2;

    char *ipport = malloc(size);
    char *message = malloc(size + 22);
    snprintf(ipport, size, "%s:%s", conf->servers[0].ip, conf->servers[0].port);
    snprintf(message, size + 22, "Preparing on server : %s:%s",
             conf->servers[0].ip, conf->servers[0].port);
    log_message(message);

    if (strcmp(req->host, conf->servers[0].ip) != 0
        && strcmp(req->host, conf->servers[0].server_name->data) != 0
        && strcmp(req->host, ipport) != 0)
    {
        log_message("SERVER : INVALID HOST");
        conf->error = 1;
    }
    free(ipport);
    free(message);

    req->status_code = "200";
    req->reason_phrase = "OK";

    check_error(conf, req);
}

void server_loop(int listening_sock, struct config *conf, char *config)
{
    while (1)
    {
        int client_sock = accept(listening_sock, NULL, NULL);
        if (client_sock == -1)
            continue;

        char buff[3000];
        ssize_t nb_read;
        size_t total_read = 0;
        while ((nb_read =
                    recv(client_sock, buff + total_read, 3000 - total_read, 0))
               > 0)
        {
            char *end_header = strstr(buff + total_read, "\r\n\r\n");
            if (end_header != NULL)
            {
                size_t index = end_header - buff + 4;
                total_read = index;
                break;
            }
        }
        if (nb_read == -1)
        {
            close(client_sock);
            continue;
        }
        struct request *req = parse_request(buff);
        check_req(conf, req);

        char *response = create_response(req, config);

        log_message(response);
        send_res(client_sock, response);
        free(response);
    }
}

int server(char *config)
{
    struct config *conf = parse_configuration(config);
    char *ip = conf->servers[0].ip;
    char *port = conf->servers[0].port;
    int listening_sock = create_and_bind(ip, port);
    if (listening_sock == -1)
    {
        config_destroy(conf);
        return 2;
    }

    if (listen(listening_sock, 30) == -1)
    {
        config_destroy(conf);
        return 3;
    }

    server_loop(listening_sock, conf, config);
    return 0;
}
