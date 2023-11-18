#define _POSIX_C_SOURCE 200809L

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "parse_request.h"
#include "logger/logger.h"

struct request *parse_request(char response[])
{
    struct request *req = calloc(1,sizeof(struct request));
    char *line = strtok(response, " ");
    //char *field = strtok(line, " ");

    req->method = strdup(line);
    line = strtok(0, " ");
    req->target = strdup(line);
    line = strtok(0, "\r\n");
    req->http = strdup(line);
    req->err = 0;
    while ((line = strtok(0,"\r\n"))!= NULL)
    {
        if (strncmp(line, "Host: ", 6) == 0)
        {
            req->host = line + 6;
        }
        //si besoin d'add d'autre fields
    }
    line = strtok(0,"");
    req->body = line;

    ////
    /// if le fopen pour content length renvoit null aLORS CHECK LA ERRNO
    /// 404
    ///
    /// if (errno == EACCES )
    ///  alors reutrn error 403
    ///
    //    printf("%s\n", http);
    if (strcmp(req->http, "HTTP/1.1") != 0)
    {
        req->err = 1;
        log_message("error not http/1.1");
        // error 400
    }

    /*printf("request target : %s\n", req->target);
    printf("request http : %s\n", req->http);
    printf("request method : %s\n", req->method);
    printf("request host : %s\n", req->host);
    printf("request body : %s\n", req->body);*/

    return req;
}