#ifndef CREATE_RESPONSE_H
#define CREATE_RESPONSE_H

#include "../config/config.h"
#include "parse_request.h"

int file_char_count(struct server_config *server_conf, struct request *req);

char *body_from_file(struct server_config *server_conf, struct request *req);

char *create_response(struct request *req, struct server_config *server_conf);

#endif /* !CREATE_RESPONSE_H */
