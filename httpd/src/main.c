#include "utils/prettyprinter/prettyprinter.h"
#include "config/config.h"
#include "utils/string/string.h"
#include "server/server.h"
#include "http/parse_request.h"
#include "logger/logger.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main(int argc, char *argv[])
{
    if (argc == 0)
        return 1;
    struct config *c = parse_configuration(argv[1]);
    setup_logging(c->log, c->log_file);
    //log_message("TESSSSSSSSSSSSSSST");
    if (argc == 2)
    {
        //char buff[] = "GET / HTTP/1.1\r\nHost: 127.0.0.1:4243\r\nUser-Agent: curl/7.88.1\r\nAccept: */*\r\n\r\nTOTOTOTOTOTOTO";
        //parse_request(buff);
        log_message("starting server");
        server(argv[1]);
    }

    printf("le log_file est : %s\n", c->log_file);
    printf("log est : %s\n", c->log ? "true" : "false");
    printf("le pid_file est : %s\n", c->pid_file);
    for (size_t i = 0; i < c->nb_servers;i++)
    {
        printf("------ vhost numero : %ld\n", i);
        printf("le server_name est : %s\n", c->servers[i].server_name->data);
        printf("le port est : %s\n", c->servers[i].port);
        printf("l'ip est : %s\n", c->servers[i].ip);
        printf("le default_file est : %s\n", c->servers[i].default_file);
        printf("le root_dir est : %s\n", c->servers[i].root_dir);
    }
    config_destroy(c);
    return 0;
}