#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"


/**
 * TODO
*/
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (fs == NULL || inp == NULL) return -1;

    int ips = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    int total = (fs -> superblock).s_isize * ips;       
    if (inumber < 1 || inumber > total) return -1;

    int sector = INODE_START_SECTOR + (inumber - 1) / ips;
    struct inode buf[DISKIMG_SECTOR_SIZE / sizeof(struct inode)];
    if (diskimg_readsector(fs -> dfd, sector, buf) != DISKIMG_SECTOR_SIZE) return -1;

    *inp = buf[(inumber - 1) % ips];
    if (((inp -> i_mode) & IALLOC) == 0) return -1; // Verifica que el inodo esté asignado

    return 0;
}

/**
 * TODO
*/
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
    if (fs == NULL || inp == NULL || blockNum < 0) return -1;

    int is_large = ((inp -> i_mode) & ILARG) != 0;
    if (!is_large) {
        if (blockNum >= 8) return -1;

        int blk = inp -> i_addr[blockNum];
        return (blk != 0 ? blk : -1);
    }

    int ptrs = DISKIMG_SECTOR_SIZE / sizeof(uint16_t); 
    if (blockNum < 7 * ptrs) {
        int idx = blockNum / ptrs;
        int off = blockNum % ptrs;
        int iblk = inp -> i_addr[idx];
        if (iblk == 0) return -1;

        uint16_t buf[DISKIMG_SECTOR_SIZE / sizeof(uint16_t)];
        if (diskimg_readsector(fs -> dfd, iblk, buf) != DISKIMG_SECTOR_SIZE) return -1;

        int res = buf[off];
        return (res != 0 ? res : -1);
    }

    int rem = blockNum - 7 * ptrs;
    int max = ptrs * ptrs;
    if (rem < 0 || rem >= max) return -1;

    int dbl = inp -> i_addr[7];
    if (dbl == 0) return -1;

    uint16_t buf1[DISKIMG_SECTOR_SIZE / sizeof(uint16_t)];
    if (diskimg_readsector(fs -> dfd, dbl, buf1) != DISKIMG_SECTOR_SIZE) return -1;

    int i1 = rem / ptrs;
    int off1 = rem % ptrs;
    int iblk = buf1[i1];
    if (iblk == 0) return -1;

    uint16_t buf2[DISKIMG_SECTOR_SIZE / sizeof(uint16_t)];
    if (diskimg_readsector(fs -> dfd, iblk, buf2) != DISKIMG_SECTOR_SIZE) return -1;

    int res = buf2[off1];
    return (res != 0 ? res : -1);
}

int inode_getsize(struct inode *inp) {
    return (((inp -> i_size0) << 16) | (inp -> i_size1)); 
}
