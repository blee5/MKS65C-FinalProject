/*
 * util.c
 * 
 * Utility functions.
 */

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"

#include <errno.h>
#include <stdio.h>
#include <string.h>

void report_error(const char *msg)
{
    fprintf(stderr, "%s[Error] %s%s: %s\n", KRED, KNRM, msg, strerror(errno));
}
