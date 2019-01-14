#include <limits.h>

struct packet
{
    char method[255];
    char status[255];
    char file[PATH_MAX];
    char version[255];
    struct hashtable *fields; 
    char *body;
};


int send_resp(int sockfd, struct packet *response);
int prep_resp(int sockfd, struct packet *request, struct packet *response);
int parse_req(struct packet *ht, char *request);
char *get_field(struct packet *p, char *key);
void clear_packet(struct packet *p);
