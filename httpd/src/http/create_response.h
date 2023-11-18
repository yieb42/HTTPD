#ifndef CREATE_RESPONSE_H
#define CREATE_RESPONSE_H

int file_char_count(char *path);

char *body_from_file(char *path, char *body);

char *create_response(char *response, char *status_code, char *reason_phrase, int content_length, char *method, char *bodyy);

#endif /* !CREATE_RESPONSE_H */
