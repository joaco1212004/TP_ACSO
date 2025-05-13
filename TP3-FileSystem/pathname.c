
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    assert(fs != NULL && pathname != NULL);
    if (pathname[0] != '/')
        return -1;
    int curr = ROOT_INUMBER;
    if (strcmp(pathname, "/") == 0)
        return curr;
    char pathbuf[1024];
    strncpy(pathbuf, pathname + 1, sizeof(pathbuf) - 1);
    pathbuf[sizeof(pathbuf) - 1] = '\0';
    char *saveptr;
    char *token = strtok_r(pathbuf, "/", &saveptr);
    struct direntv6 dirEnt;
    while (token != NULL) {
        if (directory_findname(fs, token, curr, &dirEnt) < 0)
            return -1;
        curr = dirEnt.d_inumber;
        token = strtok_r(NULL, "/", &saveptr);
    }
    return curr;
}
