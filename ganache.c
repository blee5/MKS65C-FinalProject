/*
 * ganache.c
 * 
 * A simple and bad HTTP server.
 */

#include <arpa/inet.h>
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
#include "requests.h"
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
    char *port = "80";

    signal(SIGCHLD, sighandler);
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

void child_server(int sockfd)
{
    char *buf = malloc(MAX_REQ_SIZE + 1);
    char *temp;
    int status;
    struct packet request = {0};
    struct packet response = {0};
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
                report_error("could not read from socket");
            }
            exit(0);
        }

        parse_req(&request, temp);
        printf("%s %s %s - " , request.method, request.file, request.version);

        prep_resp(sockfd, &request, &response);
        send_resp(sockfd, &response);
    }
}
