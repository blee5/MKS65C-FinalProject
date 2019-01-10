struct packet
{
    char method[255];
    char status[255];
    char file[255];
    char version[255];
    struct hashtable *fields; 
};

int parse_req(struct packet *ht, char *request);
char *get_field(struct packet *p, char *key);
