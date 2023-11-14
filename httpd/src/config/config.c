#define _POSIX_C_SOURCE 200809L

#include "config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char *remove_line_return(char *conf)
{
    //conf[strcspn(conf, "\n")] = 0;
    return conf;
}

struct config *parse_configuration(const char *path)
{
    struct config *conf = malloc(sizeof(struct config));
    struct server_config *server = malloc(sizeof(struct server_config));
    FILE *fp = fopen(path,"r");
    if (!fp)
    {
        return NULL;
    }
    char *buff = NULL;
    //char *line = NULL;
    size_t len = 0;
    ssize_t read;
    conf->nb_servers = 0;
    int num_mand = 0;
    //while(fgets(buff,1024,fp) != NULL)
    while((read = getline(&buff,&len,fp)) != -1)
    {
        if (!strcmp(buff,"[global]\n"))
        {
            continue;
        }
        if(!strcmp(buff,"\n"))
        {
            continue;
        }
        if (!strcmp(buff,"[[vhosts]]\n"))
        {
            conf->nb_servers++;
            server = realloc(server,sizeof(struct server_config) * conf->nb_servers);
            continue;
        }
        char *field = strtok(buff, " = ");
        while (field)
        {
            if(!strcmp(field,"log_file"))
            {
                field = strtok(NULL, " = ");
                conf->log_file = remove_line_return(strdup(field));
            }
            if(!strcmp(field,"pid_file"))
            {
                num_mand++;
                field = strtok(NULL, " = ");
                conf->pid_file = remove_line_return(strdup(field));
            }
            if(!strcmp(field,"log"))
            {
                field = strtok(NULL, " = ");
                if (!strcmp(field, "true\n"))
                {
                    conf->log = true;
                }
                else if(!strcmp(field, "false\n"))
                {
                    conf->log = false;
                }
            }
            if(!strcmp(field,"server_name"))
            {
                num_mand++;
                field = strtok(NULL," = ");
                server[conf->nb_servers - 1].server_name = malloc(sizeof(struct string));
                server[conf->nb_servers - 1].server_name->data = remove_line_return(strdup(field));
                server[conf->nb_servers - 1].server_name->size = strlen(field);
            }
            if(!strcmp(field,"port"))
            {
                num_mand++;
                field = strtok(NULL, " = ");
                server[conf->nb_servers - 1].port = remove_line_return(strdup(field));
            }
            if(!strcmp(field, "ip"))
            {
                num_mand++;
                field = strtok(NULL, " = ");
                server[conf->nb_servers - 1].ip = remove_line_return(strdup(field));
            }
            if(!strcmp(field,"default_file"))
            {
                field = strtok(NULL, " = ");
                server[conf->nb_servers - 1].default_file = remove_line_return(strdup(field));
            }
            if(!strcmp(field,"root_dir"))
            {
                num_mand++;
                field = strtok(NULL," = ");
                server[conf->nb_servers - 1].root_dir = remove_line_return(strdup(field));
            }
            field = strtok(NULL, " = ");
        }
    }
    conf->servers = server;
    if (num_mand != 5)
    {
        return NULL;
    }

    fclose(fp);
    return conf;
}

void config_destroy(struct config *config)
{
    free(config->servers->server_name->data);
    free(config->servers->server_name);
    free(config->servers->port);
    free(config->servers->ip);
    free(config->servers->root_dir);
    free(config->servers->default_file);
    free(config->servers);
    free(config->pid_file);
    free(config->log_file);
    free(config);
}
