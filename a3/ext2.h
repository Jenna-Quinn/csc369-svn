#include <stdint.h>

/**
 * The ext2 superblock contains information about the layout of the file system.
 * It always starts at byte 1024 (0x400 in hex) and is 1024 bytes long.
 *
 * Field descriptions from <http://wiki.osdev.org/Ext2#Base_Superblock_Fields>
 */
struct ext2_superblock {
    uint32_t num_inodes;                // Total number of inodes in file system
    uint32_t num_blocks;                // Total number of blocks in file system
    uint32_t num_blocks_superuser;      // Number of blocks reserved for superuser
    uint32_t num_blocks_free;           // Total number of unallocated blocks
    uint32_t num_inodes_free;           // Total number of unallocated inodes
    uint32_t superblock_bn;             // Block number of the block containing the superblock
    uint32_t blocksize_offset;          // Block size offset (i.e. 1024 << blocksize_offset == block size)
    uint32_t fragmentsize_offset;       // Fragment size offset (i.e. 1024 << fragmentsize_offset == block size)
    uint32_t num_blocks_per_group;      // Number of blocks in each block group
    uint32_t num_fragments_per_group;   // Number of fragments in each block group
    uint32_t num_inodes_per_group;      // Number of inodes in each block group
    uint32_t last_mount_time;           // Last mount time (in POSIX time)
    uint32_t last_write_time;           // Last written time (in POSIX time)
    uint16_t num_mounts_since_check;    // Number of times the volume has been mounted since its last consistency check (fsck)
    uint16_t num_mounts_before_check;   // Number of mounts allowed before a consistency check (fsck) must be done
    uint16_t ext2_signature;            // Ext2 signature (0xef53), used to help confirm the presence of Ext2 on a volume
    uint16_t file_system_state;         // File system state:
                                        //   1: File system is clean
                                        //   2: File system has errors
    uint16_t error_handler;             // What to do when an error is detected:
                                        //   1: Ignore the error (continue on)
                                        //   2: Remount file system as read-only
                                        //   3: Kernel panic
    uint16_t version_minor;             // Minor portion of version (combine with Major portion below to construct full version field)
    uint32_t last_check_time;           // POSIX time of last consistency check (fsck)
    uint32_t check_interval;            // Interval (in POSIX time) between forced consistency checks (fsck)
    uint32_t os_id;                     // Operating system ID from which the filesystem on this volume was created:
                                        //   0: Linux
                                        //   1: GNU HURD
                                        //   2: MASIX
                                        //   3: FreeBSD
                                        //   4: Other "Lites" (BSD4.4-Lite derivatives)
    uint32_t version_major;             // Major portion of version (combine with Minor portion above to construct full version field)
    uint16_t reserved_user_id;          // User ID that can use reserved blocks
    uint16_t reserved_group_id;         // Group ID that can use reserved blocks

    char padding[940];                  // (Unused)
};

/*
 * Operator macros on struct ext2_superblock.
 *
 * BLOCK_SIZE:	  return offset to next/previous header
 * FRAGMENT_SIZE: return next/previous header
 */

#define BLOCKSIZE(sb)	 (1024 << (sb)->blocksize_offset)
#define FRAGMENTSIZE(sb) (1024 << (sb)->fragmentsize_offset)

/**
 * A block group is a group of inodes and data blocks.
 *
 * Field descriptions from <http://wiki.osdev.org/Ext2#Block_Group_Descriptor>
 */
struct ext2_block_group {
    uint32_t block_usage_map_addr; // Block address of block usage bitmap
    uint32_t inode_usage_map_addr; // Block address of inode usage bitmap
    uint32_t inode_table_addr;     // Starting block address of inode table
    uint16_t num_blocks_free;      // Number of unallocated blocks in group
    uint16_t num_inodes_free;      // Number of unallocated inodes in group
    uint16_t num_directories;      // Number of directories in group

    char padding[14];              // (Unused)
};

/**
 * An entire ext2 disk
 */
struct ext2_disk {
    uint8_t *bytes;
    struct ext2_superblock *superblock;
    struct ext2_block_group **block_groups;
};

struct ext2_disk *ext2_read_disk(const char *filename);
struct ext2_block_group **ext2_read_block_groups(struct ext2_disk *disk);
