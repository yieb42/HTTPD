#define _POSIX_C_SOURCE 200809L

#include <netdb.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "server.h"
#include "config/config.h"
#include "http/parse_request.h"
#include "utils/string/string.h"
#include "logger/logger.h"
#include "http/create_response.h"

int create_and_bind(const char *node, const char *service)
{
    struct addrinfo hints= {0};
    //memset(&hints, 0, sizeof(struct addrinfo));
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
        sock =
                socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == -1)
            continue;

        int yes = 1;
        setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&yes,sizeof(yes));

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

int server(char *config) {
    struct config *conf = parse_configuration(config);
    char *ip = conf->servers[0].ip;
    char *port = conf->servers[0].port;
    size_t size = strlen(conf->servers[0].ip) + strlen(conf->servers[0].port)+2;
    char *ipport = strdup(conf->servers[0].ip);
    //ipport = "";
    ipport = realloc(ipport,size);
    //strcat(ipport,conf->servers[0].ip);
    strcat(ipport,":");
    strcat(ipport,conf->servers[0].port);
    char *message = strdup("Preparing server on ");
    size_t message_size = strlen(message) + strlen(ipport) + 1;  // +1 for the null terminator
    message = realloc(message, message_size);  // Assign the result of realloc back to message
    strcat(message, ipport);
    log_message(message);
    int listening_sock = create_and_bind(ip, port);
    if (listening_sock == -1)
        return 2;

    if (listen(listening_sock, 30) == -1)
        return 3;

    while (1) {
        int client_sock = accept(listening_sock, NULL, NULL);
        if (client_sock == -1)
            continue;

        char buff[300];
        ssize_t nb_read;
        size_t total_read = 0;
        //printf("LAAAAAAAAAAAAAAAAAAAA");
        while ((nb_read =
                        recv(client_sock, buff + total_read, 300 - total_read, 0))
               > 0) {
            char *end_header = strstr(buff + total_read, "\r\n\r\n");
            if (end_header != NULL) {
                size_t index = end_header - buff + 4;
                total_read = index;
                break;
            }
            total_read += nb_read;
            //printf("LAAA");
        }
        if (nb_read == -1) {
            close(client_sock);
            continue;
        }
        //recuperer la string buff qui est la reponse
        struct request *req = parse_request(buff);
        //printf("IP:PORT -> %s\n", ipport);
        if (strcmp(req->host,conf->servers[0].ip)!= 0 && strcmp(req->host,conf->servers[0].server_name->data)!= 0 &&
                strcmp(req->host,ipport) != 0)
        {
            //GERER L'ERREUR INVALID HOST
            log_message("SERVER : INVALID HOST");
            printf("INVALID HOST");
        }
        free(ipport);
        free(message);
        int content_length = file_char_count(req->target);

        char response[2000];
        char *status_code = "200";
        char *reason_phrase = "OK";
        if (req->err == 1)
        {
            status_code = "400";
            reason_phrase = "Bad Request";
        }
        char bodyy[500] = { 0 };
        body_from_file(req->target, bodyy);
        create_response(response, status_code, reason_phrase, content_length, req->method, bodyy);
        printf("%s\n",response);
        log_message(response);
        ssize_t nb_sent;
        size_t total_sent = 0;
        while ((nb_sent = send(client_sock, response , strlen(response) - total_sent , MSG_NOSIGNAL)) > 0)
        {
            total_sent += nb_sent;
        }

        //printf("LA RESPONSE : %s\n", buff);
        close(client_sock);
    }
    return 0;
}