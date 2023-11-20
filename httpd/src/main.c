#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config/config.h"
#include "daemon/daemon.h"
#include "http/parse_request.h"
#include "logger/logger.h"
#include "server/server.h"
#include "utils/prettyprinter/prettyprinter.h"
#include "utils/string/string.h"

int main(int argc, char *argv[])
{
    if (argc == 0)
        return 1;
    else if (argc == 2)
    {
        struct config *c = parse_configuration(argv[1]);
        setup_logging(c->log, c->log_file);
        log_message("starting server");
        server(argv[1]);
        config_destroy(c);
        return 0;
    }
    else if (argc == 3)
    {
        if (!strcmp("--dry-run", argv[1]))
        {
            log_message("dry run on server");
            struct config *c = parse_configuration(argv[2]);
            if (c->error != 0)
            {
                config_destroy(c);
                fprintf(stderr, "ERROR PARSING CONFIGURATION FILE\n");
                return 2;
            }
            config_destroy(c);
            return 0;
        }
    }
    else if (argc == 4)
    {
        if (!strcmp(argv[2], "start"))
        {
            log_message("start daemon");
            struct config *c = parse_configuration(argv[3]);
            if (c->log_file == NULL)
            {
                setup_logging(c->log, "HTTPd.log ");
            }
            else
                setup_logging(c->log, c->log_file);
            char *pid_file = malloc(strlen(c->pid_file) + 1);
            strcpy(pid_file, c->pid_file);
            config_destroy(c);
            return start(pid_file, argv[3]);
        }
        else if (!strcmp(argv[2], "stop"))
        {
            log_message("stop daemon");
            struct config *c = parse_configuration(argv[3]);
            if (c->log_file == NULL)
            {
                setup_logging(c->log, "HTTPd.log ");
            }
            else
                setup_logging(c->log, c->log_file);
            char *pid_file = malloc(strlen(c->pid_file) + 1);
            strcpy(pid_file, c->pid_file);
            config_destroy(c);
            return stop(pid_file);
        }
        else if (!strcmp(argv[2], "restart"))
        {
            log_message("restart daemon");
            struct config *c = parse_configuration(argv[3]);
            if (c->log_file == NULL)
            {
                setup_logging(c->log, "HTTPd.log ");
            }
            else
                setup_logging(c->log, c->log_file);
            char *pid_file = malloc(strlen(c->pid_file) + 1);
            strcpy(pid_file, c->pid_file);
            config_destroy(c);
            return restart(pid_file, argv[3]);
        }
        else
        {
            return 1;
        }
    }
    return 1;

    /*
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
    return 0;*/
}
