/*
 * ganache.c
 * 
 * A simple and bad HTTP server.
 */

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
#include "hashtable.h"
#include "files.h"
#include "util.h"

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

    port = "3003";
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
        fprintf(stderr, "could not address info: %s\n", gai_strerror(status));
        exit(1);
    }

    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sockfd < 0)
    {
        report_error("could not open socket");
        exit(1);
    }

    /* Yes, let me just use the socket that's in TIME_WAIT */
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
        report_error("could not set socket option");
        exit(1);
    } 

    status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (status != 0)
    {
        report_error("could not bind to socket");
        exit(1);
    }

    status = listen(sockfd, 128);
    if (status != 0)
    {
        report_error("could not listen to socket");
        exit(1);
    }

    free(servinfo);
    return sockfd;
}

int parse_request(struct hashtable *ht, char *request)
{
    /*
     * Parses an HTTP request and inserts key-value pairs into *ht.
     */
    char *line;
    char *key;

    /* Parse first line */
    line = strsep(&request, "\n");
    *strchr(line, '\r') = 0;
    insert(ht, "Method", strsep(&line, " "));
    insert(ht, "File", strsep(&line, " "));
    insert(ht, "Version", line);

    for (;;)
    {
        line = strsep(&request, "\n");
        *strchr(line, '\r') = 0;
        key = strsep(&line, ": "); // line now points to the value
        if (line == NULL)
        {
            break;
        }
        insert(ht, key, line);
    }
    return 0;
}

void child_server(int sockfd)
{
    char *buf = malloc(MAX_REQ_SIZE + 1);
    char *path, *body, *temp;
    int status;
    int body_length;
    struct hashtable *req_dict;
    /* Buffer overflow protection */
    buf[MAX_REQ_SIZE] = 0;
    for (;;)
    {
        memset(buf, 0, MAX_REQ_SIZE);
        temp = buf;
        status = read(sockfd, buf, MAX_REQ_SIZE);

        if (status <= 0)
        {
            /*
             * If read returned 0, the client exited normally.
             * Otherwise, an error occured. 
             */
            if (status < 0)
            {
                fprintf(stderr, "could not read from socket: %s\n", strerror(errno));
            }
            exit(0);
        }

        req_dict = init_ht();
        parse_request(req_dict, temp);
        printf("%s %s %s\n", getval(req_dict, "Method"),
                             getval(req_dict, "File"),
                             getval(req_dict, "Version"));

        path = getval(req_dict, "File");

        int fd = open_file(path);
        if (fd < 0)
        {
            char *msg = "HTTP/1.1 404 Not Found\r\nContent-Length: 22\r\n\r\n<h1>404 Not Found</h1>";
            write(sockfd, msg, strlen(msg));
        }
        else
        {
            body_length = read_file(fd, &body);
            sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nConnection: keep-alive\r\nKeep-Alive: timeout=10\r\nContent-Type: text/html\r\n\r\n", body_length);

            write(sockfd, buf, strlen(buf));
            write(sockfd, body, body_length);
            free(body);
        }
        free_ht(req_dict);
    }
}
