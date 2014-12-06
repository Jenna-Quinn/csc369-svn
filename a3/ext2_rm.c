#include <stdio.h>
#include <err.h>
#include <assert.h>
#include <search.h>
#include "ext2.h"

/**
 * Remove the target file or link
 */
void ext2_rm(struct ext2_disk *disk, const char *target_file_name) {
    // Read info about the containing directory
    char *last_segment;
    uint32_t container_inode;
    struct ext2_inode *containing_directory = ext2_traverse_path(disk, NULL, target_file_name, &last_segment, &container_inode);

    struct ext2_directory_entry *entry;
    if ((entry = ext2_read_entry_from_directory(disk, containing_directory, last_segment)) == NULL) {
        // File does not exist
        errx(1, "File with name %s does not exist", last_segment);
    } else if (IS_DIRECTORY(ext2_get_inode(disk, 0, entry->inode_addr))) {
        // Target is a directory
        errx(1, "Cannot remove directory %s", last_segment);
    } else {
        // Save the inode number
        uint32_t inode_addr = entry->inode_addr;

        // Remove the directory entry by clearing its fields
        entry->inode_addr = 0;
        entry->size = 0;
        entry->name_length = 0;
        entry->type_indicator = 0;
        entry->name = 0;

        // Decrement the hard link count
        if (--ext2_get_inode(disk, 0, inode_addr)->num_links == 0)
            // No more hard links point to the inode; free the inode and data blocks
            ext2_free_inode(disk, inode_addr);
    }
}

int main(int argc, char *argv[]) {
    char *disk_image_path;
    char *target_file_name;
    struct ext2_disk *disk;

    if (argc != 3) {
        errx(1, "USAGE: ext2_rm <disk_image> <target_file>\n");
    }

    disk_image_path = argv[1];
    target_file_name = argv[2];

    // Read the disk from file
    disk = ext2_read_disk(disk_image_path);

    // Ensure the superblock has a block size of 1024 bytes (since it's always 1024 in this assignment)
    assert(BLOCKSIZE(disk->superblock) == 1024);

    // Perform the remove operation
    ext2_rm(disk, target_file_name);

    return 0;
}
