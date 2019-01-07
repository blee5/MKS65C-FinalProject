#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ganache.h"

void sighandler(int signum)
{
    switch (signum)
    {
        /* Prevent zombie processes */
        case SIGCHLD:
            wait(NULL);
    }
}

int main(int argc, char **argv)
{
    char *test;
    int t = open("test.html", O_RDONLY);
    test = read_file(t);
    printf(" aa %s\n", test);

    int sockfd, connfd, addr_size;
    char *port;

    signal(SIGCHLD, sighandler);

    port = "80";
    if (argc > 1)
    {
        port = argv[1];
    }

    sockfd = setup_port(port);

    for (;;)
    {
        connfd = accept(sockfd, NULL, NULL);
        if (!fork())
        {
            close(sockfd);
            child_server(connfd);
        }
        close(connfd);
    }

    return 0;
}

int setup_port(char *port)
{
    int status, sockfd;
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, port, &hints, &servinfo);
    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sockfd < 0)
    {
        fprintf(stderr, "error creating socket: %s\n", strerror(errno));
        exit(1);
    }

    /* Tell system we're okay with using a port that's in TIME_WAIT */
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
        exit(1);
    }

    status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (status != 0)
    {
        fprintf(stderr, "error binding to socket: %s\n", strerror(errno));
        exit(1);
    }

    status = listen(sockfd, 128);
    if (status != 0)
    {
        fprintf(stderr, "error listening to socket: %s\n", strerror(errno));
        exit(1);
    }

    return sockfd;
}

void child_server(int sockfd)
{
    printf("[%d] Accepted new client!\n", getpid());
    for (;;)
    {
        /* TODO: fix this temp part */
        char buf[500];
        memset(buf, 0, 500);
        if (recv(sockfd, buf, 500, 0) == 0)
        {
            printf("[%d] Exiting\n", getpid());
            exit(0);
        }
        printf("[%d] Received request:\n", getpid());
        printf("%s\n", strtok(buf, "\r\n"));
        /* printf("%s\n", buf); */
        char *test_res = "HTTP/1.1 200 OK\nConnection: keep-alive\nKeep-Alive: timeout=10\nContent-Length: 31\nContent-Type: text/html\r\n\r\n<h1>TEST</h1><a href=\"\\\">hi</a>";
        send(sockfd, test_res, strlen(test_res), 0);
    }
}

char *read_file(int fd)
{
    size_t len = 10;
    char *buf = malloc(len);
    char *p;
    int read_res;
    while (read_res == len)
    {
        read_res = read(fd, p, 10);
        p += len;
        if (read_res < 0)
        {
            /* TODO: the server should not quit, but send a response */
            fprintf(stderr, "error reading frmo file: %s\n", strerror(errno));
            exit(1);
        }
        len *= 2;
        realloc(buf, len);
    }
    return buf;
}
