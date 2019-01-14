/*
 * requests.c
 * 
 * Deals with HTTP requests (and responses, what a misleading name)
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hashtable.h"
#include "requests.h"
#include "files.h"

int prep_resp(int sockfd, struct packet *request, struct packet *response)
{
    clear_packet(response);

    char buffer[255];
    long body_length;
    strcpy(response->file, "/www");
    strcat(response->file, request->file);
    int fd = open_file(response->file);
    
    strcpy(response->version, "HTTP/1.1");
    if (fd < 0)
    {
        switch (fd)
        {
            case -ENOENT:
                strcpy(response->status, "404 Not Found");
                strcpy(request->file, "error/404NotFound.html");
                insert(response->fields, "Content-Type", "text/html");
        }
    }
    else
    {
        strcpy(response->status, "200 OK");
        insert(response->fields, "Content-Type", get_type(request->file));
    }
    body_length = get_size(fd);
    snprintf(buffer, 255, "%ld", body_length);
    insert(response->fields, "Content-Length", buffer);
    insert(response->fields, "Keep-Alive", "timeout=10");
    return 0;
}

int send_resp(int sockfd, struct packet *response)
{
    dprintf(sockfd, "%s %s\r\n",
        response->version, response->status);

    /* Unpack dict */
    int i, size = response->fields->size;
    struct node *n;
    for (i = 0; i < size; i++)
    {
        n = response->fields->lists[i];
        if (n != NULL)
        {
            do
            {
                dprintf(sockfd, "%s: %s\r\n", n->key, n->value);
            }
            while ((n = n->next) != NULL);
        }
    }

    write(sockfd, "\r\n", 2);
    write_file(sockfd, response->file);
    printf("%s\n", response->status);
    return 0;
}

int parse_req(struct packet *p, char *request)
{
    /*
     * Parses an HTTP request and inserts key-value pairs into *ht.
     */
    char *line;
    char *key;
    char *temp;

    /* Clear any preexisting data  */
    clear_packet(p);

    struct hashtable *ht = p->fields;

    /* Parse first line */
    line = strsep(&request, "\n");
    if ((temp = strchr(line, '\r')) != NULL)
    {
        *temp = 0;
    }
    strcpy(p->method, strsep(&line, " "));
    strcpy(p->file, strsep(&line, " "));
    strcpy(p->version, line);

    for (;;)
    {
        line = strsep(&request, "\n");
        if ((temp = strchr(line, '\r')) != NULL)
        {
            *temp = 0;
        }
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
