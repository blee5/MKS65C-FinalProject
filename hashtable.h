#include <stddef.h>

struct hashtable *init_ht();
unsigned long hash(const char *str);
char *getval(struct hashtable *ht, const char *key);
void insert(struct hashtable *ht, const char *key, const char *value);
void free_ht(struct hashtable *ht);
