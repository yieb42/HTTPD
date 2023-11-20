#include "create_response.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../config/config.h"

int file_char_count(char *server_conf, struct request *req)
{
    struct config *c = parse_configuration(server_conf);
    char *root_dir = malloc(strlen(c->servers[0].root_dir) + 1);
    strcpy(root_dir, c->servers[0].root_dir);
    char *default_file = NULL;
    if (c->servers[0].default_file)
    {
        default_file = malloc(strlen(c->servers[0].default_file) + 1);
        strcpy(default_file, c->servers[0].default_file);
    }
    else
    {
        default_file = malloc(11);
        strcpy(default_file, "index.html");
    }
    root_dir = realloc(root_dir, strlen(root_dir) + strlen(req->target) + strlen(default_file) + 1);
    strcat(root_dir, req->target);
    strcat(root_dir, default_file);
    // printf("LE PATH TO BODY : %s\n", root_dir);
    FILE *fd = fopen(root_dir, "r");
    if (fd == NULL) {
        req->err = 2;
        return 0;
    }
    int i = 0;
    int chr;
    while ((chr = fgetc(fd)))
    {
        if (chr == EOF)
            break;
        i++;
    }
    free(root_dir);
    free(default_file);
    config_destroy(c);
    return i;
    fclose(fd);
}

char *body_from_file(char *server_conf, char *body,struct request *req)
{
    struct config *c = parse_configuration(server_conf);
    char *root_dir = c->servers[0].root_dir;
    char *default_file = c->servers[0].default_file;
    if (default_file == NULL)
    {
        default_file = "index.html";
    }
    root_dir = realloc(root_dir, strlen(root_dir) + strlen(req->target) + strlen(default_file) + 1);
    strcat(root_dir, req->target);
    strcat(root_dir, default_file);
    FILE *fd = fopen(root_dir, "r");
    if (fd == NULL)
    {
        return NULL;
    }
    char buff[1024];
    while (fgets(buff, 1024, fd) != NULL)
    {
        strcat(body, buff);
    }
    return body;
}

int int_length(int num, int base) {
    if (num == 0)
        return 1;

    if (num < 0)
        num = -num;
    int j = 0;
    while (num != 0) {
        j++;
        num = num / base;
    }
    return j;
}

char *create_response(struct request *req, char *config)
{
    char *response;
    char date[35];
    time_t t;
    struct tm *tmp;
    time(&t);
    tmp = gmtime(&t);
    int content_length = file_char_count(config,req);
    strftime(date, sizeof(date), "%a, %d %b %Y %T %Z", tmp);
    if (!strcmp(req->method, "GET"))
    {
        char *version = "HTTP/1.1";
        char bodyy[500] = { 0 };
        body_from_file(config, bodyy,req);
        size_t size = strlen(version) + 1 + strlen(req->status_code) + 1 + strlen(req->reason_phrase) + 2 + 6 + strlen(date) +2 + 16 +
                int_length(content_length,10)+2+17+4+ strlen(bodyy) + 1;
        response = malloc(size);
        sprintf(response,
                "%s %s %s\r\nDate: %s\r\nContent length: %d\r\nConnection: "
                "close\r\n\r\n%s",
                version, req->status_code, req->reason_phrase, date, content_length,
                bodyy);
        return response;
    }
    else if (!strcmp(req->method, "HEAD"))
    {
        char *version = "HTTP/1.1";
        size_t size = strlen(version) + 1 + strlen(req->status_code) + 1 + strlen(req->reason_phrase) + 2 + 6 + strlen(date) +2 + 16 +
                      int_length(content_length,10)+2+17+4 + 1;
        response = malloc(size);
        sprintf(response,
                "%s %s %s\r\nDate: %s\r\nContent length: %d\r\nConnection: "
                "close\r\n\r\n",
                version, req->status_code, req->reason_phrase, date, content_length);
        return response;
    }
    else
    {
        char *version = "HTTP/1.1";
        char *status_cde = "405";
        char *reason_phras = "Method Not Allowed";
        size_t size = strlen(version) + 1 + strlen(req->status_code) + 1 + strlen(req->reason_phrase) + 2 + 6 +
                strlen(date) +2 + 16 + int_length(content_length,10)+2+17+4 + 1;
        response = malloc(size);
        sprintf(response,
                "%s %s %s\r\nDate: %s\r\nContent length: %d\r\nConnection: "
                "close\r\n\r\n",
                version, status_cde, reason_phras, date, content_length);
        return response;
    }
}