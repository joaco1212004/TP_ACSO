#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int directory_findname(struct unixfilesystem *fs, const char *name, int dirinumber, struct direntv6 *dirEnt) {
    assert(fs != NULL && name != NULL && dirEnt != NULL);
    char buf[DISKIMG_SECTOR_SIZE];
    int block = 0;
    while (1) {
        int bytes = file_getblock(fs, dirinumber, block, buf);
        if (bytes < 0) {
            return -1;
        }
        if (bytes == 0) {
            break;
        }
        int count = bytes / sizeof(struct direntv6);
        struct direntv6 *entries = (struct direntv6 *)buf;
        for (int i = 0; i < count; i++) {
            if (entries[i].d_inumber == 0)
                continue;
            char namebuf[sizeof(entries[i].d_name) + 1];
            memcpy(namebuf, entries[i].d_name, sizeof(entries[i].d_name));
            namebuf[sizeof(entries[i].d_name)] = '\0';
            if (strcmp(name, namebuf) == 0) {
                *dirEnt = entries[i];
                return 0;
            }
        }
        block++;
    }
    return -1;
}
