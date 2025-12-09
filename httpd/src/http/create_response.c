#define _POSIX_C_SOURCE 200809L

#include "create_response.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../config/config.h"

char *set_path(struct server_config *c, struct request *req)
{
    char *root_dir = malloc(strlen(c->root_dir) + 1);
    strcpy(root_dir, c->root_dir);
    char *default_file = NULL;
    if (strcmp(req->target, "/") == 0)
    {
        if (c->default_file)
        {
            default_file = malloc(strlen(c->default_file) + 1);
            strcpy(default_file, c->default_file);
        }
        else
        {
            default_file = malloc(11);
            strcpy(default_file, "index.html");
        }
        root_dir = realloc(root_dir,
                           strlen(root_dir) + strlen(req->target)
                               + strlen(default_file) + 1);
        strcat(root_dir, req->target);
        strcat(root_dir, default_file);
    }
    else
    {
        if (req->target[strlen(req->target) - 1] == '/')
        {
            if (c->default_file)
            {
                default_file = malloc(strlen(c->default_file) + 1);
                strcpy(default_file, c->default_file);
            }
            else
            {
                default_file = malloc(11);
                strcpy(default_file, "index.html");
            }
            root_dir = realloc(root_dir,
                               strlen(root_dir) + strlen(req->target)
                                   + strlen(default_file) + 1);
            strcat(root_dir, req->target);
            strcat(root_dir, default_file);
        }
        else
        {
            root_dir =
                realloc(root_dir, strlen(root_dir) + strlen(req->target) + 1);
            strcat(root_dir, req->target);
        }
    }
    free(default_file);
    return root_dir;
}

int file_char_count(struct server_config *c, struct request *req)
{
    char *root_dir = set_path(c, req);
    FILE *fd = fopen(root_dir, "r");
    if (fd == NULL)
    {
        req->err = 3;
        free(root_dir);
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
    fclose(fd);
    return i;
}

char *body_from_file(struct server_config *c, struct request *req)
{
    char *root_dir = set_path(c, req);
    FILE *fd = fopen(root_dir, "r");
    if (fd == NULL)
    {
        req->err = 3;
        free(root_dir);
        return NULL;
    }

    fseek(fd, 0, SEEK_END);
    size_t file_size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    char *body = malloc(file_size + 1);
    if (body == NULL)
    {
        req->err = 3;
        fclose(fd);
        free(root_dir);
        return NULL;
    }

    size_t bytesRead = fread(body, 1, file_size, fd);
    if (bytesRead != file_size)
    {
        req->err = 3;
        fclose(fd);
        free(body);
        free(root_dir);
        return NULL;
    }

    body[file_size] = '\0';
    free(root_dir);
    fclose(fd);
    return body;
}

static int int_length(int num, int base)
{
    if (num == 0)
        return 1;

    if (num < 0)
        num = -num;
    int j = 0;
    while (num != 0)
    {
        j++;
        num = num / base;
    }
    return j;
}

size_t get_size(char *version, struct request *req, char *date,
                int content_length)
{
    return strlen(version) + 1 + strlen(req->status_code) + 1
        + strlen(req->reason_phrase) + 2 + 6 + strlen(date) + 2 + 16
        + int_length(content_length, 10) + 2 + 17 + 4 + 1;
}

static char *get_date(void)
{
    char date[35] = { 0 };
    time_t t;
    struct tm *tmp;
    time(&t);
    tmp = gmtime(&t);
    strftime(date, sizeof(date), "%a, %d %b %Y %T %Z", tmp);
    char *ret = strdup(date);
    return ret;
}

static int check_errors(struct request *req)
{
    if (req->err == 1)
    {
        req->status_code = "400";
        req->reason_phrase = "Bad Request";
        req->content_length = 0;
        return 1;
    }
    else if (req->err == 2)
    {
        req->status_code = "203";
        req->reason_phrase = "Forbidden";
        req->content_length = 0;
        return 1;
    }
    else if (req->err == 3)
    {
        req->status_code = "404";
        req->reason_phrase = "Not Found";
        req->content_length = 0;
        return 1;
    }
    return 0;
}

char *head_method(struct request *req, int content_length, char *date)
{
    char *response;
    char *version = "HTTP/1.1";
    size_t size = get_size(version, req, date, content_length);
    response = malloc(size);
    sprintf(response,
            "%s %s %s\r\nDate: %s\r\nContent-Length: %d\r\nConnection: "
            "close\r\n\r\n",
            version, req->status_code, req->reason_phrase, date,
            content_length);
    return response;
}

static char *get_method(struct server_config *c, struct request *req, int content_length,
                        char *date)
{
    char *response;
    char *version = "HTTP/1.1";
    char *bodyy = body_from_file(c, req);
    int sizee = 0;
    if (bodyy == NULL)
    {
        req->err = 3;
        check_errors(req);
        return head_method(req, content_length, date);
    }
    else
    {
        sizee = strlen(bodyy);
    }
    size_t size = strlen(version) + 1 + strlen(req->status_code) + 1
        + strlen(req->reason_phrase) + 2 + 6 + strlen(date) + 2 + 16
        + int_length(content_length, 10) + 2 + 17 + 4 + sizee + 1;
    response = malloc(size);
    if (bodyy)
    {
        sprintf(response,
                "%s %s %s\r\nDate: %s\r\nContent-Length: %d\r\nConnection: "
                "close\r\n\r\n%s",
                version, req->status_code, req->reason_phrase, date,
                content_length, bodyy);
    }
    free(bodyy);
    return response;
}

char *create_response(struct request *req, struct server_config *c)
{
    char *response;
    char *date = get_date();
    int content_length = file_char_count(c, req);
    int err = check_errors(req);
    if (err == 1)
    {
        char *version = "HTTP/1.1";
        size_t size = get_size(version, req, date, content_length);
        response = malloc(size);
        sprintf(response,
                "%s %s %s\r\nDate: %s\r\nContent-Length: %d\r\nConnection: "
                "close\r\n\r\n",
                version, req->status_code, req->reason_phrase, date,
                content_length);
    }
    if (!strcmp(req->method, "GET"))
    {
        response = get_method(c, req, content_length, date);
    }
    else if (!strcmp(req->method, "HEAD"))
    {
        response = head_method(req, content_length, date);
    }
    else
    {
        char *version = "HTTP/1.1";
        req->status_code = "405";
        req->reason_phrase = "Method Not Allowed";
        req->content_length = 0;
        size_t size = get_size(version, req, date, content_length);
        response = malloc(size);
        sprintf(response,
                "%s %s %s\r\nDate: %s\r\nContent-Length: %d\r\nConnection: "
                "close\r\n\r\n",
                version, req->status_code, req->reason_phrase, date,
                req->content_length);
    }
    free(date);
    return response;
}
