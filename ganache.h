int main(int argc, char **argv);
void sighandler(int signum);
void child_server(int sockfd);
int read_file(const char *filename, char **dest);
int setup_port(char *port);
