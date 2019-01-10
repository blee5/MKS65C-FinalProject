/*
 * hashtable.c
 * 
 * Implementation of a hash table (dictionary)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

/*
 * Size of internal array in hash table.
 */
#define DEF_SIZE 1024

struct node
{
    char *key;
    char *value;
    struct node *next;
};

struct hashtable
{
    ssize_t size;
    struct node **lists;
};

struct hashtable *init_ht()
{
    // TODO: error handling if allocation fails
    struct hashtable *ht = malloc(sizeof(struct hashtable));
    ht->lists = calloc(sizeof(struct node*), DEF_SIZE);
    ht->size = DEF_SIZE;
    return ht;
}

unsigned long hash(const char *str)
{
    /* djb2 algorithm (http://www.cse.yorku.ca/~oz/hash.html) */
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

size_t get_index(struct hashtable *ht, const char *key)
{
    /* Helper function to calculate index of a linked list */
    unsigned long h = hash(key);
    size_t index = h % ht->size; 
    return index;
}

char *getval(struct hashtable *ht, const char *key)
{
    /*
     * Get value from a key-value pair.
     * Returns NLUL if not found.
     */
    size_t i = get_index(ht, key);
    struct node *n;

    if (ht->lists[i] == NULL)
    {
        return NULL;
    }

    for (n = ht->lists[i]; n != NULL; n = n->next)
    {
        if (strcmp(n->key, key) == 0)
        {
            return n->value;
        }
    }
    return NULL;
}

void insert(struct hashtable *ht, const char *key, const char *value)
{
    /*
     * Inserts a key-value pair. If the key already exists, it updates the value.
     */
    size_t i = get_index(ht, key);
    struct node *n;
    if (ht->lists[i] == NULL)
    {
        ht->lists[i] = malloc(sizeof(struct node));
        n = ht->lists[i];
    }
    else
    {
        for (n = ht->lists[i]; n->next != NULL; n = n->next)
        {
            if (strcmp(n->key, key) == 0)
            {
                /* Modify existing value instead*/
                goto end;
            }
        }
        n->next = malloc(sizeof(struct node));
        n = n->next;
    }
    end:
    n->key = strdup(key);
    n->value = strdup(value);
    n->next = NULL;
}

void free_list(struct node *n)
{
    /* Helper function to free individual linked lists */
    if (n->next)
    {
        free_list(n->next);
    }
    free(n->key);
    free(n->value);
    free(n);
}

void free_ht(struct hashtable *ht)
{
    /*
     * Completely frees a hash table.
     */
    size_t size = ht->size, i = 0;
    struct node *list;
    while (i < size)
    {
        list = ht->lists[i++];
        if (list != NULL)
        {
            free_list(list);
        }
    }
    free(ht->lists);
    free(ht);
}
