#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "ext2.h"


/**
 * Get the number of block groups via the given superblock
 */
int _get_num_block_groups(struct ext2_superblock *superblock) {
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
 * Get the next directory entry.
 */
struct ext2_directory_entry *_next_directory_entry(struct ext2_disk *disk, uint32_t data_block_num, struct ext2_directory_entry *prev) {
    unsigned long byte_addr;
    if (prev == NULL) {
        byte_addr = BLOCKSIZE(disk->superblock) * data_block_num;
    } else {
        // Take `prev` byte address relative to the beginning of the file and add its size with the name,
        // rounding up to a multiple of 4
        byte_addr = ((uintptr_t) prev) - ((uintptr_t) disk->bytes) + (sizeof(prev) + prev->name_length + 3) & ~0x03;
    }

    return (struct ext2_directory_entry *) &disk->bytes[byte_addr];
}

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
 * Read the list of block group descriptors from the block group descriptor table.
 *
 * See <http://wiki.osdev.org/Ext2#Block_Group_Descriptor_Table>
 */
struct ext2_block_group **ext2_read_block_groups(struct ext2_disk *disk) {
    // Determine the number of block groups
    int num_block_groups = _get_num_block_groups(disk->superblock);

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
 * Get the inode given the block number (zero-indexed) and the inode number (one-indexed).
 */
struct ext2_inode *ext2_get_inode(struct ext2_disk *disk, uint32_t block_addr, uint32_t inode_addr) {
    // Find the byte address of the inode table
    int byte_addr = BLOCKSIZE(disk->superblock) * disk->block_groups[block_addr]->inode_table_bn;

    return (struct ext2_inode *) &disk->bytes[byte_addr + (inode_addr - 1) * disk->superblock->inode_size];
}

uint32_t ext2_inode_addr(struct ext2_disk *disk, struct ext2_inode *inode) {
    return (uint32_t) (((uintptr_t) inode) - ((uintptr_t) disk) / disk->superblock->inode_size);
}

/**
 * Read the directory containing the item with the given path (i.e. traverse each path segment until the last),
 * starting at the given current working directory.
 * Exit with an error if a directory along the path could not be found.
 */
struct ext2_inode *ext2_traverse_path(struct ext2_disk *disk, struct ext2_inode *cwd, const char *file_path, char **last_segment) {
    if (file_path[0] == '/') {
        // Start from the root path
        struct ext2_inode *root_directory = ext2_get_inode(disk, 0, 2);
        if (file_path[1] == '\0') {
            // Path is just "/"; return the root directory
            *last_segment = "";
            return root_directory;
        }

        // Otherwise, recurse using root_directory as cwd
        return ext2_traverse_path(disk, root_directory, file_path + 1, last_segment);
    } else {
        // file_path is relative to cwd; search for first path segment in cwd

        // Copy file_path into str
        char *str = malloc(strlen(file_path) + 1);
        strcpy(str, file_path);

        // Get the first path segment from str
        char *segment = strsep(&str, "/");

        if (str == NULL) {
            // `segment` is the last path segment; return this cwd
            *last_segment = segment;
            return cwd;
        }

        // Find the directory inode in this directory
        struct ext2_directory_entry *entry_found = ext2_read_entry_from_directory(disk, cwd, segment);
        struct ext2_inode *inode_found = ext2_get_inode(disk, 0, entry_found->inode_addr);

        if (inode_found == NULL) {
            // Could not find inode in directory
            errx(1, "Could not find file or directory %s", segment);
        } else if (IS_DIRECTORY(inode_found) && str != NULL) {
            // Found a directory, and is not last path segment
            // Recurse with file_found as cwd
            return ext2_traverse_path(disk, inode_found, str, last_segment);
        } else {
            // Found the desired file; return it
            return inode_found;
        }
    }
}

struct ext2_directory_entry *ext2_read_entry_from_directory(struct ext2_disk *disk, struct ext2_inode *cwd, const char *name) {
    struct ext2_directory_entry *entry = NULL;

    // Iterate over the 12 direct block pointers
    int i;
    for (i = 0; i < 12; i++) {
        while ((entry = _next_directory_entry(disk, cwd->direct_blocks[i], entry))->inode_addr != 0) {
            // Check if the name matches
            if (strcmp(ENTRY_NAME(entry), name) == 0)
                // Name matches; return this entry
                return entry;
        }
    }

    return entry;
}

struct ext2_directory_entry *ext2_create_entry(struct ext2_disk *disk, struct ext2_directory_entry *entry, uint32_t inode_addr, struct ext2_inode *inode, char *name) {
    entry->inode_addr = inode_addr;
    entry->name_length = (uint8_t) strlen(name);

    // Copy name into name field
    strcpy(&entry->name, name);
}

uint32_t _get_free_inode_addr(struct ext2_disk *disk) {
    uint16_t *inode_map = (uint16_t *) &disk->bytes[BLOCKSIZE(disk->superblock) * disk->block_groups[0]->inode_usage_map_bn];
    uint32_t i;
    for (i = 0; i < disk->superblock->num_inodes_per_group; i++) {
        if ((*inode_map & (1 << i)) == 0) {
            return i + 1;
        }
    }
    return 0;
}

struct ext2_inode *ext2_create_inode(struct ext2_disk *disk, struct ext2_inode *cwd, const char *last_segment, uint32_t *inode_addr) {
    // Find a free inode via the usage bitmap
    if ((*inode_addr = _get_free_inode_addr(disk)) == 0)
        errx(1, "Could not find a free inode");

    return ext2_get_inode(disk, 0, *inode_addr);
}

int ext2_write_data(struct ext2_disk *disk, struct ext2_inode *file, FILE *source_file) {

}

int ext2_write_directory_data(struct ext2_disk *disk, struct ext2_inode *containing_directory, struct ext2_directory_entry *entry) {
    struct ext2_inode *inode = ext2_get_inode(disk, 0, entry->inode_addr);

    uint32_t container_inode_addr = ext2_inode_addr(disk, containing_directory);
}

void ext2_set_inode_in_use(struct ext2_disk *disk, uint32_t inode_addr) {
    uint16_t *inode_map = (uint16_t *) &disk->bytes[BLOCKSIZE(disk->superblock) * disk->block_groups[0]->inode_usage_map_bn];
    *inode_map |= 1 << inode_addr;
}

/**
 * Read the disk image with the given filename into a structure.
 */
struct ext2_disk *ext2_read_disk(const char *filename) {
    // Assert some ext2 facts before continuing
    assert(sizeof(struct ext2_superblock) == 1024);
    assert(sizeof(struct ext2_block_group) == 32);
    assert(sizeof(struct ext2_inode) == 128);

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
