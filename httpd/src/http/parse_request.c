#define _POSIX_C_SOURCE 200809L

#include "parse_request.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../logger/logger.h"

struct request *parse_request(char response[])
{
    struct request *req = calloc(1, sizeof(struct request));
    char *line = strtok(response, " ");

    req->method = strdup(line);
    line = strtok(0, " ");
    req->target = strdup(line);
    line = strtok(0, "\r\n");
    req->http = strdup(line);
    req->err = 0;
    while ((line = strtok(0, "\r\n")) != NULL)
    {
        if (strncmp(line, "Host: ", 6) == 0)
        {
            req->host = line + 6;
        }
    }
    line = strtok(0, "");
    req->body = line;

    if (strcmp(req->http, "HTTP/1.1") != 0)
    {
        req->err = 1;
        log_message("error not http/1.1");
    }
    if (req->target[0] != '/')
    {
        req->err = 1;
    }

    return req;
}
