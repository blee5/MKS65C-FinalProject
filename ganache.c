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
#include "hashtable.h"

#define MAX_REQ_SIZE 500

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

    printf("Setup complete, starting server on port %s\n", port);
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
        fprintf(stderr, "error getting address info: %s\n", gai_strerror(status));
        exit(1);
    }

    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sockfd < 0)
    {
        report_error("error opening socket");
        exit(1);
    }

    /* Yes, let me just use the socket that's in TIME_WAIT */
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
        report_error("error setting socket option");
        exit(1);
    } 

    status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (status != 0)
    {
        report_error("error binding to socket");
        exit(1);
    }

    status = listen(sockfd, 128);
    if (status != 0)
    {
        report_error("error listening to socket");
        exit(1);
    }

    free(servinfo);
    return sockfd;
}

void child_server(int sockfd)
{
    int status;
    printf("[%d] Accepted new client!\n", getpid());
    for (;;)
    {
        char request[MAX_REQ_SIZE];
        status = read(sockfd, request, MAX_REQ_SIZE);

        if (status <= 0)
        {
            if (status < 0)
            {
                fprintf(stderr, "error reading from socket: %s\n", strerror(errno));
            }
            printf("[%d] Exiting\n", getpid());
            exit(0);
        }

        char buf[500];
        printf("[%d] Received request:\n", getpid());
        printf("%s\n", strtok(request, "\r\n"));
        
        char *body;
        int body_length;

        /* TODO: Replace this placeholder response */

        /* This is just temporary please don't be mad I just wanted to see a chicken */
        if (strcmp(request, "GET /chicken.png HTTP/1.1") == 0)
        {
            body_length = read_file("chicken.png", &body);
            sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nConnection: keep-alive\r\nKeep-Alive: timeout=10\r\nContent-Type: image/png\r\n\r\n", body_length);
        }
        else
        {
            body_length = read_file("index.html", &body);
            sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nConnection: keep-alive\r\nKeep-Alive: timeout=10\r\nContent-Type: text/html\r\n\r\n", body_length);
        }

        write(sockfd, buf, strlen(buf));
        write(sockfd, body, body_length);
        free(body);
    }
}

int read_file(const char *filename, char **dest)
{
    /*
     * Opens a file and writes it to a string pointed by dest.
     * MAKE SURE TO FREE THE STRING!!
     *
     * Returns size of the file in bytes.
     * Returns -1 if an error occurs.
     */
    int fd = open(filename, O_RDONLY);
    struct stat s;
    int filesize;

    if (fd < 0)
    {
        return -1;
    }

    fstat(fd, &s);
    filesize = s.st_size;

    *dest = malloc(filesize);
    if (*dest == NULL)
    {
        report_error("could not allocate memory");
        return -1;
    }
    if (read(fd, *dest, filesize) < 0)
    {
        report_error("could not read file");
        return -1;
    }
    return filesize;
}

void report_error(const char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}
