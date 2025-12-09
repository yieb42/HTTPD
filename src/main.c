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

int launch_daemon(char *arg2, char *arg3)
{
    if (!strcmp(arg2, "start"))
    {
        log_message("start daemon");
        struct config *c = parse_configuration(arg3);
        if (c->log_file == NULL)
        {
            setup_logging(c->log, "HTTPd.log ");
        }
        else
            setup_logging(c->log, c->log_file);
        char *pid_file = malloc(strlen(c->pid_file) + 1);
        strcpy(pid_file, c->pid_file);
        config_destroy(c);
        return start(pid_file, arg3);
    }
    else if (!strcmp(arg2, "stop"))
    {
        log_message("stop daemon");
        struct config *c = parse_configuration(arg3);
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
    else if (!strcmp(arg2, "restart"))
    {
        log_message("restart daemon");
        struct config *c = parse_configuration(arg3);
        if (c->log_file == NULL)
        {
            setup_logging(c->log, "HTTPd.log ");
        }
        else
            setup_logging(c->log, c->log_file);
        char *pid_file = malloc(strlen(c->pid_file) + 1);
        strcpy(pid_file, c->pid_file);
        config_destroy(c);
        return restart(pid_file, arg3);
    }
    else
    {
        return 1;
    }
}

int main(int argc, char *argv[])
{
    if (argc == 0)
    {
        perror("Usage : ./httpd [--dry-run] [-a (start | stop | reload | "
               "restart)] <server.conf>");
        return 1;
    }
    else if (argc == 2)
    {
        struct config *c = parse_configuration(argv[1]);
        if (c == NULL)
        {
            perror("error in parse file doesnt exist");
            return 1;
        }
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
                perror("ERROR PARSING CONFIGURATION FILE\n");
                return 2;
            }
            config_destroy(c);
            return 0;
        }
    }
    else if (argc == 4)
    {
        return launch_daemon(argv[2], argv[3]);
    }
    perror("Usage : ./httpd [--dry-run] [-a (start | stop | reload | restart)] "
           "<server.conf>");
    return 1;
}
