#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

/**
 * TODO
 */
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    // Obtener el inodo
    struct inode inp;
    if (inode_iget(fs, inumber, &inp) < 0)
        return -1;

    // Traducir bloque lógico a físico
    int phys = inode_indexlookup(fs, &inp, blockNum);
    if (phys < 0)
        return -1;

    // Leer sector completo
    int r = diskimg_readsector(fs->dfd, phys, buf);
    if (r != DISKIMG_SECTOR_SIZE)
        return -1;

    // Calcular bytes válidos según tamaño del archivo
    int fsize = inode_getsize(&inp);
    int off = blockNum * DISKIMG_SECTOR_SIZE;
    if (off >= fsize) {
        // EOF: bloque más allá del final
        return 0;
    } else if (off + DISKIMG_SECTOR_SIZE <= fsize) {
        return DISKIMG_SECTOR_SIZE;
    } else {
        return fsize - off;
    }
}

