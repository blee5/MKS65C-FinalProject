#include <stddef.h>

int main(int argc, char **argv);
int setup_port(char *port);
void sighandler(int signum);
void child_server(int sockfd);
int read_file(const char *filename, char **dest);
void report_error(const char *msg);
