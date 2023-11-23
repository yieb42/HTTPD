#define _POSIX_C_SOURCE 200809L

#include "daemon.h"

#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../server/server.h"

int start(char *pid_path, char *server_conf)
{
    int f = open(pid_path, O_RDONLY);
    if (f == -1)
    {
        printf("FICHIER PID_FILE EXISTE PAS\n");
    }
    else
    {
        char buff[32];
        if (read(f, buff, 32))
        {
            if (kill(atoi(buff), 0) == 0)
            {
                close(f);
                free(pid_path);
                return 1;
            }
        }
        close(f);
    }
    FILE *fp = fopen(pid_path, "w");
    if (fp == NULL)
    {
        printf("ERREUR FOPEN PIDFILE");
        free(pid_path);
        return -1;
    }
    int val = fork();
    if (val == -1)
    {
        printf("FORK ERROR");
        fclose(fp);
        free(pid_path);
        return -1;
    }
    if (!val)
    {
        fprintf(fp, "%d", getpid());
        fclose(fp);
        free(pid_path);
        server(server_conf);
    }
    else
    {
        printf("%d\n", val);
        free(pid_path);
        fclose(fp);
    }
    return 0;
}

int stop(char *pid_path)
{
    FILE *fd = fopen(pid_path, "r");
    if (fd == NULL)
    {
        free(pid_path);
        return 0;
    }
    if (fd != NULL)
    {
        char *line = NULL;
        size_t len = 0;
        if (getline(&line, &len, fd))
        {
            if (kill(atoi(line), 0) == 0)
            {
                int ppid = atoi(line);
                if (line)
                    free(line);
                remove(pid_path);
                free(pid_path);
                fclose(fd);
                kill(ppid, SIGINT);
                return 0;
            }
            if (line)
                free(line);
        }
    }
    fclose(fd);
    remove(pid_path);
    free(pid_path);
    return 0;
}

int restart(char *pid_path, char *server_conf)
{
    FILE *fd = fopen(pid_path, "r");
    if (fd == NULL)
    {
        char *cpy_pid_path = malloc(strlen(pid_path) + 1);
        strcpy(cpy_pid_path, pid_path);
        stop(pid_path);
        return start(cpy_pid_path, server_conf);
    }
    else
    {
        char *line = NULL;
        size_t len = 0;
        if (getline(&line, &len, fd))
        {
            if (kill(atoi(line), 0) == 0)
            {
                if (line)
                    free(line);
                char *cpy_pid_path = malloc(strlen(pid_path) + 1);
                strcpy(cpy_pid_path, pid_path);
                fclose(fd);
                stop(pid_path);
                return start(cpy_pid_path, server_conf);
            }
            else
            {
                if (line)
                    free(line);
                char *cpy_pid_path = malloc(strlen(pid_path) + 1);
                strcpy(cpy_pid_path, pid_path);
                fclose(fd);
                stop(pid_path);
                return start(cpy_pid_path, server_conf);
            }
        }
        else
        {
            if (line)
                free(line);
            char *cpy_pid_path = malloc(strlen(pid_path) + 1);
            strcpy(cpy_pid_path, pid_path);
            fclose(fd);
            stop(pid_path);
            return start(cpy_pid_path, server_conf);
        }
    }
}
