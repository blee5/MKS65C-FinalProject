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


struct hashtable *init_ht();
unsigned long hash(const char *str);
char *getval(struct hashtable *ht, const char *key);
void insert(struct hashtable *ht, const char *key, const char *value);
void free_ht(struct hashtable *ht);
