#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Bad usage: %s <pipe_name>\n", argv[0]);
        return 1;
    }

    struct epoll_event ev;
    struct epoll_event events[100];

    int pipefd = open(argv[1], O_RDONLY);
    if (pipefd == -1)
    {
        perror("open");
        return 1;
    }
    int nfds;
    int epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll create");
        return 1;
    }

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = pipefd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, pipefd, &ev) == -1)
    {
        perror("epoll ctl for epollfd");
        return 1;
    }
    char buff[32] = { 0 };
    while (1)
    {
        nfds = epoll_wait(epollfd, events, 100, -1);
        if (nfds == -1)
        {
            perror("epoll wait");
            return 1;
        }
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].events & EPOLLIN)
            {
                ssize_t readd = read(events[i].data.fd, buff, 32);
                if (readd == -1)
                {
                    perror("error on read");
                    return 1;
                }

                buff[readd] = '\0';

                if (strcmp(buff, "pong") == 0)
                {
                    printf("ping!\n");
                }
                else if (strcmp(buff, "ping") == 0)
                {
                    printf("pong!\n");
                }
                else if (strcmp(buff, "quit") == 0)
                {
                    printf("quit\n");
                    close(pipefd);
                    close(epollfd);
                    return 0;
                }
                else
                {
                    printf("Unknown: %s\n", buff);
                }
            }
        }
    }
}