#define _POSIX_C_SOURCE 200809L

#include "server.h"

#include <netdb.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "../config/config.h"
#include "../http/create_response.h"
#include "../http/parse_request.h"
#include "../logger/logger.h"

int create_and_bind(const char *node, const char *service)
{
    struct addrinfo hints = { 0 };
    // memset(&hints, 0, sizeof(struct addrinfo));
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
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO | SO_REUSEADDR, &yes, sizeof(yes));

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

int server(char *config)
{
    struct config *conf = parse_configuration(config);
    int erreur = 0;
    if (conf->error == 1)
    {
        erreur = 1;
    }
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
    //config_destroy(conf);
    while (1)
    {
        int client_sock = accept(listening_sock, NULL, NULL);
        if (client_sock == -1)
            continue;

        char buff[300];
        ssize_t nb_read;
        size_t total_read = 0;
        while ((nb_read =
                    recv(client_sock, buff + total_read, 300 - total_read, 0))
               > 0)
        {
            char *end_header = strstr(buff + total_read, "\r\n\r\n");
            if (end_header != NULL)
            {
                size_t index = end_header - buff + 4;
                total_read = index;
                break;
            }
            total_read += nb_read;
        }
        if (nb_read == -1)
        {
            close(client_sock);
            continue;
        }
        struct request *req = parse_request(buff);
        // printf("IP:PORT -> %s\n", ipport);
        //int content_length = file_char_count(config);
        char *ipport = strdup(conf->servers[0].ip);
        size_t size =
                strlen(conf->servers[0].ip) + strlen(conf->servers[0].port) + 2;
        ipport = realloc(ipport, size);
        strcat(ipport, ":");
        strcat(ipport, conf->servers[0].port);
//        char *cpy_ipport = malloc(strlen(ipport)+1);
//        strcpy(cpy_ipport, ipport);
        char *message = strdup("Preparing server on ");
        size_t message_size = strlen(message) + strlen(ipport) + 1;
        message = realloc(message, message_size);
        strcat(message, ipport);
        log_message(message);
//        free(ipport);
//        free(message);
        if (strcmp(req->host,conf->servers[0].ip) != 0 &&
                          strcmp(req->host,conf->servers[0].server_name->data) != 0 &&
                          strcmp(req->host,ipport) != 0)
        {
            //GERER L'ERREUR INVALID HOST
            log_message("SERVER : INVALID HOST");
            printf("INVALID HOST");
            conf->error = 1;
        }
        free(ipport);
        free(message);
        //char response[2000];
        req->status_code = "200";
        //char *status_code = "200";
        req->reason_phrase = "OK";
        //char *reason_phrase = "OK";
        if (erreur == 1 || req->err == 1)
        {
            req->status_code = "400";
            req->reason_phrase ="Bad Request";
        }
        if (req->err == 2)
        {
            req->status_code = "203";
            req->reason_phrase = "Forbidden";
        }
        char* response = create_response(req,config);
        // printf("%s\n",response);
        log_message(response);
        ssize_t nb_sent;
        size_t total_sent = 0;
        while ((nb_sent = send(client_sock, response,
                               strlen(response) - total_sent, MSG_NOSIGNAL))
               > 0)
        {
            total_sent += nb_sent;
        }

        // printf("LA RESPONSE : %s\n", buff);
        close(client_sock);
    }
    return 0;
}