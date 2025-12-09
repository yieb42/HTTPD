#define _POSIX_C_SOURCE 200809L

#include "server.h"

#include <netdb.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "config/config.h"
#include "http/create_response.h"
#include "http/parse_request.h"
#include "logger/logger.h"

static volatile int boucle = 1;

void graceful_shutdown(int arg)
{
    (void)arg;
    boucle = 0;
}

void sigchld_handler(int s)
{
    (void)s;
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    errno = saved_errno;
}

int create_and_bind(const char *node, const char *service)
{
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *res = NULL;
    if (getaddrinfo(node, service, &hints, &res) != 0)
        return -1;

    struct addrinfo *p;
    int sock = -1;

    for (p = res; p; p = p->ai_next)
    {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == -1)
            continue;

        int yes = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        {
            close(sock);
            sock = -1;
            continue;
        }

        if (bind(sock, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sock);
            sock = -1;
            continue;
        }

        break; // Successfully bound
    }

    freeaddrinfo(res);
    return sock;
}

void send_res(int client_sock, char *response)
{
    ssize_t nb_sent;
    size_t total_sent = 0;
    size_t len = strlen(response);
    while (total_sent < len)
    {
        nb_sent = send(client_sock, response + total_sent, len - total_sent,
                       MSG_NOSIGNAL);
        if (nb_sent == -1)
            break;
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
        req->err = 1;
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

static void check_req(struct config *conf, struct request *req, int server_idx)
{
    struct server_config *s_conf = &conf->servers[server_idx];
    
    // Basic validation logic - can be expanded
    // For now, we just log and set 200 OK if no errors
    
    char *message = malloc(256);
    if (message)
    {
        snprintf(message, 256, "Handling request on server : %s:%s",
                 s_conf->ip, s_conf->port);
        log_message(message);
        free(message);
    }

    req->status_code = "200";
    req->reason_phrase = "OK";

    check_error(conf, req);
}

void handle_client(int client_sock, struct config *conf, char *config_path, int server_idx)
{
    (void)config_path;
    char buff[3000];
    ssize_t nb_read;
    size_t total_read = 0;

    // Simple read loop (naive implementation for now)
    while ((nb_read = recv(client_sock, buff + total_read, 3000 - total_read - 1, 0)) > 0)
    {
        total_read += nb_read;
        buff[total_read] = '\0';
        if (strstr(buff, "\r\n\r\n"))
            break;
        if (total_read >= 2999) 
            break;
    }

    if (total_read > 0)
    {
        struct request *req = parse_request(buff);
        if (req)
        {
            check_req(conf, req, server_idx);
            char *response = create_response(req, &conf->servers[server_idx]);
            if (response)
            {
                log_message(response);
                send_res(client_sock, response);
                free(response);
            }
            request_destroy(req);
        }
    }
    close(client_sock);
}

int server(char *config_path)
{
    struct config *conf = parse_configuration(config_path);
    if (!conf)
        return 1;

    int *sockets = malloc(conf->nb_servers * sizeof(int));
    if (!sockets)
    {
        config_destroy(conf);
        return 1;
    }

    int max_fd = 0;
    for (size_t i = 0; i < conf->nb_servers; i++)
    {
        sockets[i] = create_and_bind(conf->servers[i].ip, conf->servers[i].port);
        if (sockets[i] == -1)
        {
            fprintf(stderr, "Failed to bind server %zu (%s:%s)\n", i, conf->servers[i].ip, conf->servers[i].port);
            // Cleanup previous sockets? For now just fail.
            free(sockets);
            config_destroy(conf);
            return 2;
        }
        if (listen(sockets[i], 30) == -1)
        {
            perror("listen");
            free(sockets);
            config_destroy(conf);
            return 3;
        }
        if (sockets[i] > max_fd)
            max_fd = sockets[i];
    }

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        free(sockets);
        config_destroy(conf);
        return 1;
    }

    struct sigaction action;
    action.sa_handler = graceful_shutdown;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGINT, &action, NULL) == -1)
    {
        perror("sigaction");
        free(sockets);
        config_destroy(conf);
        return 1;
    }

    log_message("Server started. Waiting for connections...");

    fd_set read_fds;
    while (boucle)
    {
        FD_ZERO(&read_fds);
        for (size_t i = 0; i < conf->nb_servers; i++)
        {
            FD_SET(sockets[i], &read_fds);
        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            if (boucle) perror("select");
            continue;
        }

        for (size_t i = 0; i < conf->nb_servers; i++)
        {
            if (FD_ISSET(sockets[i], &read_fds))
            {
                int client_sock = accept(sockets[i], NULL, NULL);
                if (client_sock == -1)
                {
                    perror("accept");
                    continue;
                }

                pid_t pid = fork();
                if (pid == -1)
                {
                    perror("fork");
                    close(client_sock);
                }
                else if (pid == 0)
                {
                    // Child process
                    for (size_t j = 0; j < conf->nb_servers; j++)
                        close(sockets[j]); // Close listening sockets in child
                    free(sockets);
                    
                    handle_client(client_sock, conf, config_path, i);
                    
                    config_destroy(conf);
                    exit(0);
                }
                else
                {
                    // Parent process
                    close(client_sock);
                }
            }
        }
    }

    for (size_t i = 0; i < conf->nb_servers; i++)
        close(sockets[i]);
    free(sockets);
    config_destroy(conf);
    return 0;
}
