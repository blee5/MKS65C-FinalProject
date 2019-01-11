/*
 * requests.c
 * 
 * Deals with HTTP requests (and responses, what a misleading name)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hashtable.h"
#include "requests.h"
#include "files.h"

int prep_resp(int sockfd, struct packet *request, struct packet *response)
{
    char buffer[100]; //  
    long body_length;
    int fd = open_file(request->file);
    clear_packet(response);
    
    strcpy(response->version, "HTTP/1.1");
    if (fd < 0)
    {
        strcpy(response->status, "404 Not Found");
    }
    else
    {
        body_length = read_file(fd, &response->body);
        /* Convert body_length in long to a string */
        snprintf(buffer, 100, "%ld", body_length);
        insert(response->fields, "Content-Length", buffer);
        strcpy(response->status, "200 OK");
    }
    insert(response->fields, "Keep-Alive", "timeout=10");
    return 0;
}

int send_resp(int sockfd, struct packet *response)
{
    /* temp placeholder, TODO: unpack dict to str */
    char *body_length = getval(response->fields, "Content-Length"); 
    dprintf(sockfd, "%s %s\r\n",
        response->version, response->status);
    dprintf(sockfd, "%s: %s\r\n",
        "Content-Length", body_length);
    write(sockfd, "\r\n", 2);
    write(sockfd, response->body, strtol(body_length, NULL, 10));
    return 0;
}

int parse_req(struct packet *p, char *request)
{
    /*
     * Parses an HTTP request and inserts key-value pairs into *ht.
     */
    char *line;
    char *key;

    /* Clear any preexisting data  */
    clear_packet(p);

    struct hashtable *ht = p->fields;

    /* Parse first line */
    line = strsep(&request, "\n");
    *strchr(line, '\r') = 0;
    strcpy(p->method, strsep(&line, " "));
    strcpy(p->file, strsep(&line, " "));
    strcpy(p->version, line);

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

char *get_field(struct packet *p, char *key)
{
    return getval(p->fields, key);
}

void clear_packet(struct packet *p)
{
    /*
     * Clears any data in a packet and frees memory appropriately
     */
    if (p->fields)
    {
        free_ht(p->fields);
    }
    memset(p, 0, sizeof(struct packet));
    p->fields = init_ht();
}
