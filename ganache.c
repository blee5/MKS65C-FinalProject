#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int sockfd, status, addr_size, conn_fd;
    char *port;
    struct addrinfo hints, *servinfo;
    struct sockaddr_storage client_addr;

    port = "80";
    if (argc > 1)
    {
        port = argv[1];
    }

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

    status = listen(sockfd, 1);
    if (status != 0)
    {
        fprintf(stderr, "error listening to socket: %s\n", strerror(errno));
        exit(1);
    }

    addr_size = sizeof client_addr;
    for (;;)
    {
        conn_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
        printf("Accepted a connection.\n");
        if (!fork())
        {
            close(sockfd);
            for (;;)
            {
                char buf[500];
                memset(buf, 0, 500);
                recv(conn_fd, buf, 500, 0);
                printf("%s\n", buf);
                char *test_res = "HTTP/1.1 404 Not Found\nContent-Length: 17\nContent-Type: text/html\nConnection: Closed\r\n\r\n<h1>TEST</h1>";
                send(conn_fd, test_res, strlen(test_res), 0);
                exit(0);
            }
        }
        close(conn_fd);
    }

    return 0;
}
