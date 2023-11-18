#include "prettyprinter.h"
#include "./config/config.h"

#include <stdio.h>

void pretty_print(char *path)
{
    struct config *c = parse_configuration(path);
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
}