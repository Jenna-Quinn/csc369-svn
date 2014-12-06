#include <stdio.h>
#include <err.h>
#include <assert.h>
#include <string.h>
#include "ext2.h"

/**
 * Create a hard link
 */
void ext2_ln(struct ext2_disk *disk, const char *source_path, const char *target_path) {
    // Read info about the containing directories
    char *source_last_segment;
    char *target_last_segment;
    uint32_t source_container_inode;
    uint32_t target_container_inode;
    struct ext2_inode *source_containing_directory = ext2_traverse_path(disk, NULL, source_path, &source_last_segment, &source_container_inode);
    struct ext2_inode *target_containing_directory = ext2_traverse_path(disk, NULL, target_path, &target_last_segment, &target_container_inode);

    struct ext2_directory_entry *link;
    if ((link = ext2_read_entry_from_directory(disk, source_containing_directory, source_last_segment))->inode_addr != 0) {
        // File already exists
        errx(1, "File or directory with name %s already exists", source_last_segment);
    }
    struct ext2_directory_entry *target;
    if ((target = ext2_read_entry_from_directory(disk, target_containing_directory, target_last_segment))->inode_addr == 0) {
        // File does not exist
        errx(1, "Target file or directory with name %s does not exist", target_last_segment);
    }

    // Set link entry attributes
    link->inode_addr = target->inode_addr;
    link->size = target->size;
    link->name_length = target->name_length;
    link->type_indicator = target->type_indicator;
    strcpy(&link->name, &target->name);

    // Increment target link count
    ext2_get_inode(disk, 0, link->inode_addr)->num_links++;
}

int main(int argc, char *argv[]) {
    char *disk_image_path;
    char *source_path;
    char *target_path;
    struct ext2_disk *disk;

    if (argc != 4) {
        errx(1, "USAGE: ext2_ln <disk_image> <source_path> <target_path>\n");
    }

    disk_image_path = argv[1];
    source_path = argv[2];
    target_path = argv[3];

    // Read the disk from file
    disk = ext2_read_disk(disk_image_path);

    // Ensure the superblock has a block size of 1024 bytes (since it's always 1024 in this assignment)
    assert(BLOCKSIZE(disk->superblock) == 1024);

    // Perform the link operation
    ext2_ln(disk, source_path, target_path);

    return 0;
}
