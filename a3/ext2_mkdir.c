#include <stdio.h>
#include <err.h>
#include <assert.h>
#include <search.h>
#include <string.h>
#include "ext2.h"

/**
 * Create a directory
 */
void ext2_mkdir(struct ext2_disk *disk, const char *target_directory_name) {
    if (target_directory_name == NULL || target_directory_name[0] != '/')
        errx(1, "You must provide an absolute path to a directory.");

    // Read info about the containing directory
    char *last_segment;
    uint32_t container_inode;
    struct ext2_inode *containing_directory = ext2_traverse_path(disk, NULL, target_directory_name, &last_segment, &container_inode);

    struct ext2_directory_entry *entry;
    if ((entry = ext2_read_entry_from_directory(disk, containing_directory, last_segment))->inode_addr != 0) {
        // File or directory already exists
        errx(1, "File or directory %s already exists", last_segment);
    } else {
        // Create a new inode
        uint32_t inode_addr;
        struct ext2_inode *inode = ext2_create_inode(disk, containing_directory, last_segment, &inode_addr);

        // Set directory type
        inode->type_permissions = 0x41FF;
        // Set link count
        inode->num_links = 1;

        // Set the entry fields
        entry->inode_addr = inode_addr;
        entry->name_length = (uint8_t) strlen(last_segment);

        // Copy name into name field
        strcpy(&entry->name, last_segment);

        // Write the data
        int num_bytes = ext2_write_directory_data(disk, container_inode, entry);

        // Set the entry size and type
        entry->size = (uint16_t) (sizeof(entry) + num_bytes);
        entry->type_indicator = 2;

        // Mark inode as in-use
        ext2_set_inode_in_use(disk, inode_addr);
    }
}

int main(int argc, char *argv[]) {
    char *disk_image_path;
    char *target_directory_name;
    struct ext2_disk *disk;

    if (argc != 3) {
        errx(1, "USAGE: ext2_mkdir <disk_image> <target_directory>\n");
    }

    disk_image_path = argv[1];
    target_directory_name = argv[2];

    // Read the disk from file
    disk = ext2_read_disk(disk_image_path);

    // Ensure the superblock has a block size of 1024 bytes (since it's always 1024 in this assignment)
    assert(BLOCKSIZE(disk->superblock) == 1024);

    // Perform the mkdir operation
    ext2_mkdir(disk, target_directory_name);

    return 0;
}
