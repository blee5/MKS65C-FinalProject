/*
 * files.c
 * 
 * Functions dealing with files.
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hashtable.h"
#include "util.h"

struct hashtable *filetypes = NULL;

long open_file(const char *path)
{
    /*
     * Open a file. Yay.
     *
     * If path is a directory, index.html in that directory is opened instead.
     *
     * Returns the file's descriptor on success.
     * Returns -1 if an error occurs.
     */
    char filepath[PATH_MAX + strlen(path)];
    getcwd(filepath, PATH_MAX);
    strcat(filepath, path);
    struct stat s;
    int fd;

    if ((fd = open(filepath, O_RDONLY)) < 0)
    {
        /* We don't need to report file not existing */
        if (errno != ENOENT)
        {
            report_error("could not open file");
        }
        return -errno;
    }

    fstat(fd, &s);
    /* Is directory, default to index.html */
    if (S_ISDIR(s.st_mode))
    {
        if ((fd = openat(fd, "index.html", O_RDONLY)) < 0)
        {
            report_error("could not open file");
            return -1;
        }
    }
    
    return fd;
}

long get_size(int fd)
{
    /*
     * Get size of a file in bytes.
     * Returns -1 if an error occurs.
     */
    struct stat s;

    if (fstat(fd, &s) < 0)
    {
        /* We don't need to report file not existing */
        if (errno != ENOENT)
        {
            report_error("could not open file");
        }
        return -1;
    }
    return s.st_size;
}

int write_file(int sockfd, const char *path)
{
    /*
     * Writes a file to a socket.
     */
    int CHUNK_SIZE = 500;
    char buf[CHUNK_SIZE];
    int bytes_read;
    int fd = open_file(path);
    if (fd < 0)
    {
        return -1;
    }

    do
    {
        bytes_read = read(fd, buf, CHUNK_SIZE);
        /* printf("%d\n", bytes_read); */
        /* printf("%s\n", buf); */
        if (bytes_read < 0)
        {
            report_error("could not read file");
            return -1;
        }
        write(sockfd, buf, bytes_read);
    }
    while (bytes_read == CHUNK_SIZE);

    return 0;
}

char *get_type(const char *filename)
{
    /*
     * Get content type of a file based on its extension.
     * Defaults to text/html on errors.
     */
    char *extension, *type;
    extension = strrchr(filename, '.');
    if (extension == NULL || filetypes == NULL)
    {
        return "text/html";
    }
    type = getval(filetypes, ++extension);
    return type? type: "text/html";
}

void init_types()
{
    /*
     * Reads list of MIME types from "mimetypes" and generates a dictionary based on it.
     * Currently it's hardcoded as a temporary placeholder.
     * Maaaybe I'll actually write the code for this part in the future.
     */
    int fd = open("mimetypes", O_RDONLY);
    if (fd < 0)
    {
        report_error("could not open 'mimetypes', defaulting to text/html");
        return;
    }
    filetypes = init_ht();
    insert(filetypes, "html", "text/html");
    insert(filetypes, "css", "text/css");
    insert(filetypes, "gif", "image/gif");
    insert(filetypes, "mp3", "audio/mpeg");
    insert(filetypes, "png", "image/png");
}
