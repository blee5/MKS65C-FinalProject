/* Generic utility functions */

#include <errno.h>
#include <stdio.h>
#include <string.h>

void report_error(const char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}
