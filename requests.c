/*
 * requests.c
 * 
 * Deals with HTTP requests (and responses, what a misleading name)
 */

#include <string.h>

#include "hashtable.h"
#include "requests.h"

int dict_to_resp(struct hashtable *ht, char *dest)
{
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
    if (p->fields)
    {
        free_ht(p->fields);
    }
    memset(p, 0, sizeof(struct packet));
    p->fields = init_ht();

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

