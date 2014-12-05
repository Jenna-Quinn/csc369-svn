#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ext2.h"

/**
 * Read the superblock from the given file.
 */
struct ext2_superblock *ext2_read_superblock(uint8_t *bytes) {
    // Read starting at byte 1024
    struct ext2_superblock *superblock = (struct ext2_superblock *) &bytes[1024];

    // Verify ext2 signature
    if (superblock->ext2_signature != 0xef53) {
        errx(1, "Superblock ext2 signature (%#x) does not match expected value (%#x)", superblock->ext2_signature, 0xef53);
    }

    return superblock;
}

/**
 * Get the number of block groups via the given superblock
 */
int get_num_block_groups(struct ext2_superblock *superblock) {
    // Round up integer quotient, i.e. ceil(a / b) == (a + b - 1) / b
    int num_block_groups_via_blocks =
            (superblock->num_blocks + superblock->num_blocks_per_group - 1) / superblock->num_blocks_per_group;
    int num_block_groups_via_inodes =
            (superblock->num_inodes + superblock->num_inodes_per_group - 1) / superblock->num_inodes_per_group;

    // Ensure both computed values match
    assert(num_block_groups_via_blocks == num_block_groups_via_inodes);

    return num_block_groups_via_blocks;
}

/**
 * Read the list of block group descriptors from the block group descriptor table.
 *
 * See <http://wiki.osdev.org/Ext2#Block_Group_Descriptor_Table>
 */
struct ext2_block_group **ext2_read_block_groups(struct ext2_disk *disk) {
    // Determine the number of block groups
    int num_block_groups = get_num_block_groups(disk->superblock);

    // Allocate space for the block group structure pointers
    struct ext2_block_group **block_groups = malloc(num_block_groups * sizeof(struct ext2_block_group *));

    // Read `num_block_groups` block group descriptors, starting right after the superblock (i.e. starting at byte 2048)
    int i;
    for (i = 0; i < num_block_groups; i++) {
        block_groups[i] = (struct ext2_block_group *) &disk->bytes[2048 + i * 32];
    }

    return block_groups;
}

/**
 * Read the disk image with the given filename into a structure.
 */
struct ext2_disk *ext2_read_disk(const char *filename) {
    // Assert some ext2 facts before continuing
    assert(sizeof(struct ext2_superblock) == 1024);
    assert(sizeof(struct ext2_block_group) == 32);

    // Allocate space for the disk structure
    struct ext2_disk *disk = malloc(sizeof(struct ext2_disk));

    /*
     * Map the disk image from file
     */
    // Open a file descriptor
    int fd = open(filename, O_RDWR);
    if (fd == -1) err(1, "Failed to open file %s", filename);

    // Get the file size
    struct stat st;
    fstat(fd, &st);

    // Map the file into memory
    disk->bytes = mmap(0,                       // Let the OS choose the start address
                       (size_t) st.st_size,     // Map the entire file size into memory
                       PROT_READ | PROT_WRITE,  // Read and write permissions
                       MAP_SHARED,              // Share file changes with other processes
                       fd,                      // Use the file descriptor from above
                       0);                      // Start at the beginning of the file
    if (disk->bytes == MAP_FAILED) err(1, "Failed to map file %s to memory", filename);

    // Read the superblock
    disk->superblock = ext2_read_superblock(disk->bytes);

    // Read all block groups
    disk->block_groups = ext2_read_block_groups(disk);

    return disk;
}
