#ifndef CREATE_RESPONSE_H
#define CREATE_RESPONSE_H

#include "parse_request.h"

int file_char_count(char *path, struct request *req);

char *body_from_file(char *path, struct request *req);

char *create_response(struct request *req, char *config);

#endif /* !CREATE_RESPONSE_H */
