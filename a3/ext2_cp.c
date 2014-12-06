#include <stdio.h>
#include <err.h>
#include <assert.h>
#include <stddef.h>
#include "ext2.h"

/**
 * Copy the source file to the target file or directory
 */
void ext2_cp(struct ext2_disk *disk, const char *source_file_name, const char *target_file_name) {
    // Read the source file from the native file system
    FILE *source_file = fopen(source_file_name, "r");
    if (source_file == NULL) err(1, "Failed to read source file");

    // Read info about the containing directory
    char *last_segment;
    struct ext2_inode *containing_directory = ext2_traverse_path(disk, NULL, target_file_name, &last_segment);

    struct ext2_directory_entry *file;
    if ((file = ext2_read_entry_from_directory(disk, containing_directory, last_segment)) != NULL) {
        // File already exists
        struct ext2_inode *inode = ext2_get_inode(disk, 0, file->inode_addr);
        if (IS_DIRECTORY(inode))
            // It's a directory; abort
            errx(1, "Directory with name %s already exists", last_segment);
        else
            // Overwrite the data
            ext2_write_data(disk, inode, source_file);
    } else {
        // Create a new entry
        struct ext2_directory_entry *entry = ext2_create_entry(disk, containing_directory, last_segment, NULL, NULL);

        // Create a new inode
        uint32_t inode_addr;
        struct ext2_inode *inode = ext2_create_inode(disk, containing_directory, last_segment, &inode_addr);

        // Set the inode for the entry
        entry->inode_addr = inode_addr;

        // Write the data
        ext2_write_data(disk, inode, source_file);

        // Mark inode as in-use
        ext2_set_inode_in_use(disk, inode_addr, file);
    }
}

int main(int argc, char *argv[]) {
    char *disk_image_path;
    char *source_file_path;
    char *target_file_name;
    struct ext2_disk *disk;

    if (argc != 4) {
        errx(1, "USAGE: ext2_cp <disk_image> <source_file> <target_file>\n");
    }

    disk_image_path = argv[1];
    source_file_path = argv[2];
    target_file_name = argv[3];

    // Read the disk from file
    disk = ext2_read_disk(disk_image_path);

    // Ensure the superblock has a block size of 1024 bytes (since it's always 1024 in this assignment)
    assert(BLOCKSIZE(disk->superblock) == 1024);

    // Perform the copy operation
    ext2_cp(disk, source_file_path, target_file_name);

    return 0;
}
