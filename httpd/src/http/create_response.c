#include <stdio.h>
#include <time.h>
#include <string.h>

#include "create_response.h"

int file_char_count(char *path)
{
    FILE *fd = fopen(path, "r");
    if (fd == NULL)
        return 0;
    int i = 0;
    int chr;
    while ((chr =fgetc(fd)))
    {
        if (chr == EOF)
            break;
        i++;
    }
    return i - 1;
    fclose(fd);
}

char *body_from_file(char *path, char *body)
{
    FILE *fd = fopen(path, "r");
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

char *create_response(char *response, char *status_code, char *reason_phrase, int content_length, char *method, char *bodyy)
{
    if (!strcmp(method, "GET")) {
        char *version = "HTTP/1.1";
        //char *status_code = "200";
        //char *reason_phrase = "OK";
        char date[35];
        time_t t;
        struct tm *tmp;
        time(&t);
        tmp = localtime(&t);
        strftime(date, sizeof(date), "%a, %d %b %Y %T %Z", tmp);
        sprintf(response, "%s %s %s\r\nDate: %s\r\nContent length: %d\r\nConnection: close\r\n\r\n%s", version,
                status_code, reason_phrase, date, content_length, bodyy);
        return response;
    } else if (!strcmp(method, "HEAD")) {
        char *version = "HTTP/1.1";
        //char *status_code = "200";
        //char *reason_phrase = "OK";
        char date[35];
        time_t t;
        struct tm *tmp;
        time(&t);
        tmp = localtime(&t);
        strftime(date, sizeof(date), "%a, %d %b %Y %T %Z", tmp);
        sprintf(response, "%s %s %s\r\nDate: %s\r\nContent length: %d\r\nConnection: close\r\n", version, status_code,
                reason_phrase, date, content_length);
        return response;
    } else {
        char *version = "HTTP/1.1";
        char *status_code = "405";
        char *reason_phrase = "Method Not Allowed";
        int content_length = 0;
        char date[35];
        time_t t;
        struct tm *tmp;
        time(&t);
        tmp = localtime(&t);
        strftime(date, sizeof(date), "%a, %d %b %Y %T %Z", tmp);
        sprintf(response, "%s %s %s\r\nDate: %s\r\nContent length: %d\r\nConnection: close\n", version, status_code,
                reason_phrase, date, content_length);
        return response;
    }
}