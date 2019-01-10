int main(int argc, char **argv);
int setup_port(char *port);
void sighandler(int signum);
void child_server(int sockfd);
void report_error(const char *msg);
