#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
    int sockfd, connfd;
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
    /*
     * Creates, binds, and listens to the given port.
     * Returns a file descriptor to the port created.
     */
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
        fprintf(stderr, "error creating sockfd: %s\n", strerror(errno));
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
        fprintf(stderr, "error binding to sockfd: %s\n", strerror(errno));
        exit(1);
    }

    status = listen(sockfd, 128);
    if (status != 0)
    {
        fprintf(stderr, "error listening to sockfd: %s\n", strerror(errno));
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
        if (read(sockfd, buf, 500) == 0)
        {
            printf("[%d] Exiting\n", getpid());
            exit(0);
        }
        printf("[%d] Received request:\n", getpid());
        printf("%s\n", strtok(buf, "\r\n"));

        char *body;
        int body_length;
        body_length = read_file("test.html", &body);
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nConnection: keep-alive\r\nKeep-Alive: timeout=10\r\nContent-Type: text/html\r\n\r\n", body_length);

        write(sockfd, buf, strlen(buf));
        write(sockfd, body, body_length);
    }
}

int read_file(const char *filename, char **dest)
{
    /*
     * Opens a file and writes it to a string pointed by dest.
     *
     * Returns size of the file in bytes.
     */
    int fd = open(filename, O_RDONLY);
    struct stat s;
    fstat(fd, &s);
    size_t filesize = s.st_size;
    *dest = malloc(filesize);
    read(fd, *dest, filesize);
    return filesize;
}
