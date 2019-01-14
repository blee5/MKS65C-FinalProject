long open_file(const char *path);
int write_file(int sockfd, const char *path);
char *get_type(const char *filename);
long get_size(int fd);
