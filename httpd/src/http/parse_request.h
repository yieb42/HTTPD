#ifndef PARSE_REQUEST_H
#define PARSE_REQUEST_H

struct request
{
    char *method;
    char *target;
    char *http;
    char *host;
    int content_length;
    char *body;
    int err;
};

struct request *parse_request(char response[]);
#endif /* !PARSE_REQUEST_H */
