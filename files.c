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
    char *filepath = malloc(PATH_MAX + strlen(path));
    getcwd(filepath, PATH_MAX);
    strcat(filepath, "/www");
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
        free(filepath);
        return -errno;
    }
    free(filepath);

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

int read_file(int fd, char **dest)
{
    /*
     * Reads contents file into a string pointed by**dest.
     * MAKE SURE TO FREE THE STRING!!
     *
     * Returns size of the file in bytes.
     * Returns -1 if an error occurs.
     */
    int filesize;
    struct stat s;
    *dest = NULL;

    fstat(fd, &s);
    filesize = s.st_size;

    *dest = malloc(filesize);
    if (*dest == NULL)
    {
        report_error("could not allocate memory");
        return -1;
    }
    if (read(fd, *dest, filesize) < 0)
    {
        report_error("could not read file");
        free(*dest);
        return -1;
    }

    return filesize;
}

char *get_type(const char *filename)
{
    /*
     * Get content type of a file based on its extension.
     * Defaults to text/html on errors.
     */
    char *extension, *type;
    /* Read mimetypes if it hasn't been done already */
    if (filetypes == NULL)
    {
        int fd = open("mimetypes", O_RDONLY);
        if (fd < 0)
        {
            report_error("could not open 'mimetypes', defaulting to text/html");
            return "text/html";
        }
        filetypes = init_ht();
        /* Placeholder */
        insert(filetypes, "html", "text/html");
        insert(filetypes, "gif", "image/gif");
        insert(filetypes, "mp3", "audio/mpeg");
        insert(filetypes, "png", "image/png");
    }
    extension = strrchr(filename, '.');
    if (extension == NULL)
    {
        return "text/html";
    }
    type = getval(filetypes, ++extension);
    if (type)
    {
        return type;
    }
    return "text/html";
}
