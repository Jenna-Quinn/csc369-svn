#include <stdio.h>
#include <err.h>
#include <assert.h>
#include "ext2.h"

int main(int argc, char *argv[]) {
    char *disk_image_path;
    char *source_file;
    char *target_file;
    struct ext2_disk *disk;

    if (argc != 4) {
        errx(1, "USAGE: ext2_cp <disk_image> <source_file> <target_file>\n");
    }

    disk_image_path = argv[1];
    source_file = argv[2];
    target_file = argv[3];

    // Read the disk from file
    disk = ext2_read_disk(disk_image_path);

    // Ensure the superblock has a block size of 1024 bytes
    assert(BLOCKSIZE(disk->superblock) == 1024);

    



    printf("Hello, World!\n");
    return 0;
}
