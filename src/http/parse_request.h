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
    char *status_code;
    char *reason_phrase;
};

struct request *parse_request(char response[]);

void request_destroy(struct request *req);

#endif /* !PARSE_REQUEST_H */
