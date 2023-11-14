#define _POSIX_C_SOURCE 200809L

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *remove_line_return(char *conf)
{
    conf[strcspn(conf, "\n")] = 0;
    return conf;
}

bool check_config(struct config *conf)
{
    if (conf->nb_servers == 0)
        return false;
    if (!conf->pid_file)
        return false;
    for (size_t i = 0; i < conf->nb_servers; i++)
    {
        if (!conf->servers[i].server_name || !conf->servers[i].port
            || !conf->servers[i].ip || !conf->servers[i].root_dir)
        {
            return false;
        }
    }
    return true;
}
struct config *parse_configuration(const char *path)
{
    struct config *conf = calloc(1, sizeof(struct config));
    conf->servers = malloc(sizeof(struct server_config));
    conf->log = true;
    FILE *fp = fopen(path, "r");
    if (!fp)
    {
        return NULL;
    }
    char *buff = NULL;
    size_t len = 0;
    ssize_t read;
    conf->nb_servers = 0;
    while ((read = getline(&buff, &len, fp)) != -1)
    {
        if (!strcmp(buff, "[global]\n") || !strcmp(buff, "\n"))
        {
            continue;
        }
        if (!strcmp(buff, "[[vhosts]]\n"))
        {
            conf->nb_servers++;
            if (conf->nb_servers > 1)
            {
                conf->servers =
                    realloc(conf->servers,
                            sizeof(struct server_config) * conf->nb_servers);
            }
            conf->servers[conf->nb_servers - 1].server_name =
                NULL;
            conf->servers[conf->nb_servers - 1].port = NULL;
            conf->servers[conf->nb_servers - 1].ip = NULL;
            conf->servers[conf->nb_servers - 1].root_dir = NULL;
            conf->servers[conf->nb_servers - 1].default_file = NULL;
            continue;
        }
        char *field = strtok(buff, " = ");
        while (field)
        {
            if (!strcmp(field, "log_file"))
            {
                field = strtok(NULL, " = ");
                conf->log_file = remove_line_return(strdup(field));
            }
            if (!strcmp(field, "pid_file"))
            {
                field = strtok(NULL, " = ");
                conf->pid_file = remove_line_return(strdup(field));
            }
            if (!strcmp(field, "log"))
            {
                field = strtok(NULL, " = ");
                if (!strcmp(field, "true\n"))
                {
                    conf->log = true;
                }
                else if (!strcmp(field, "false\n"))
                {
                    conf->log = false;
                }
            }
            if (!strcmp(field, "server_name"))
            {
                field = strtok(NULL, " = ");
                conf->servers[conf->nb_servers - 1].server_name =
                    calloc(1, sizeof(struct string));
                conf->servers[conf->nb_servers - 1].server_name->data =
                    remove_line_return(strdup(field));
                conf->servers[conf->nb_servers - 1].server_name->size =
                    strlen(field) - 1;
            }
            if (!strcmp(field, "port"))
            {
                field = strtok(NULL, " = ");
                conf->servers[conf->nb_servers - 1].port =
                    remove_line_return(strdup(field));
            }
            if (!strcmp(field, "ip"))
            {
                field = strtok(NULL, " = ");
                conf->servers[conf->nb_servers - 1].ip =
                    remove_line_return(strdup(field));
            }
            if (!strcmp(field, "default_file"))
            {
                field = strtok(NULL, " = ");
                conf->servers[conf->nb_servers - 1].default_file =
                    remove_line_return(strdup(field));
            }
            if (!strcmp(field, "root_dir"))
            {
                field = strtok(NULL, " = ");
                conf->servers[conf->nb_servers - 1].root_dir =
                    remove_line_return(strdup(field));
            }
            field = strtok(NULL, " = ");
        }
    }
    if (conf->nb_servers == 0)
    {
        free(conf->servers);
        free(conf->pid_file);
        free(conf->log_file);
        free(conf);
        return NULL;
    }
    if (check_config(conf) == false)
    {
        config_destroy(conf);
        return NULL;
    }

    fclose(fp);
    return conf;
}

void config_destroy(struct config *config)
{
    if (config->servers->server_name)
    {
        free(config->servers->server_name->data);
        free(config->servers->server_name);
    }
    for (size_t i = 0; i < config->nb_servers; i++)
    {
        free(config->servers[i].port);
        free(config->servers[i].ip);
        free(config->servers[i].root_dir);
        free(config->servers[i].default_file);
    }
    free(config->servers);
    free(config->pid_file);
    free(config->log_file);
    free(config);
}
