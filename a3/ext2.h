#include <stdint.h>

/**
 * An ext2 inode describes a particular file or directory.
 *
 * Field descriptions from <http://wiki.osdev.org/Ext2#Inode_Data_Structure>
 */
struct ext2_inode {
    uint16_t type_permissions;          // Type and Permissions:
                                        //   Type occupies the top 4 bits:
                                        //     0x1000 FIFO
                                        //     0x2000 Character device
                                        //     0x4000 Directory
                                        //     0x6000 Block device
                                        //     0x8000 Regular file
                                        //     0xA000 Symbolic link
                                        //     0xC000 Unix socket
                                        //   Permissions occupy the bottom 12 bits:
                                        //     0x001  Other—execute permission
                                        //     0x002  Other—write permission
                                        //     0x004  Other—read permission
                                        //     0x008  Group—execute permission
                                        //     0x010  Group—write permission
                                        //     0x020  Group—read permission
                                        //     0x040  User—execute permission
                                        //     0x080  User—write permission
                                        //     0x100  User—read permission
                                        //     0x200  Sticky Bit
                                        //     0x400  Set group ID
                                        //     0x800  Set user ID
    uint16_t user_id;                   // User ID
    uint32_t size_lower;                // Lower 32 bits of size in bytes
    uint32_t last_access_time;          // Last Access Time (in POSIX time)
    uint32_t create_time;               // Creation Time (in POSIX time)
    uint32_t last_modified_time;        // Last Modification time (in POSIX time)
    uint32_t deletion_time;             // Deletion time (in POSIX time)
    uint16_t group_id;                  // Group ID
    uint16_t num_links;                 // Count of hard links (directory entries) to this inode. When this reaches 0, the data blocks are marked as unallocated.
    uint32_t num_disk_sectors;          // Count of disk sectors (not Ext2 blocks) in use by this inode, not counting the actual inode structure nor directory entries linking to the inode.
    uint32_t flags;                     // Flags (see below)
    uint32_t os_value1;                 // Operating System Specific value #1
    uint32_t direct_blocks[12];         // Direct Block Pointers 0 to 11
    uint32_t singly_indirect_block;     // Singly Indirect Block Pointer (Points to a block that is a list of block pointers to data)
    uint32_t doubly_indirect_block;     // Doubly Indirect Block Pointer (Points to a block that is a list of block pointers to Singly Indirect Blocks)
    uint32_t triply_indirect_block;     // Triply Indirect Block Pointer (Points to a block that is a list of block pointers to Doubly Indirect Blocks)
    uint32_t generation_number;         // Generation number (Primarily used for NFS)
    uint32_t extended_attribute_block;  // Extended attribute block (File ACL).
    uint32_t size_upper_acl;            // Upper 32 bits of file size (if feature bit set) if it's a file, Directory ACL if it's a directory
    uint32_t fragment_block;            // Block address of fragment

    char padding[12];                   // (Unused)
};

/*
 * Operator macros on struct ext2_inode
 */

#define IS_DIRECTORY(inode)	(((inode)->type_permissions & 0xF000) == 0x4000)

/**
 * ext2 inodes which are directories contain many directory entries.
 */
struct ext2_directory_entry {
    uint32_t inode_addr;     // Inode address
    uint16_t size;           // Total size of this entry (Including all subfields)
    uint8_t name_length;     // Name Length least-significant 8 bits
    uint8_t type_indicator;  // Type indicator (only if the feature bit for "directory entries have file type byte" is set
    char name;               // Name characters
};

/*
 * Operator macros on struct ext2_directory_entry
 */

#define ENTRY_NAME(entry) (&(entry)->name)

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
    uint32_t first_inode_addr;          // First non-reserved inode in file system
    uint16_t inode_size;                // Size of each inode structure in bytes

    char padding[934];                  // (Unused)
};

/*
 * Operator macros on struct ext2_superblock.
 */

#define BLOCKSIZE(sb)	 (1024 << (sb)->blocksize_offset)
#define FRAGMENTSIZE(sb) (1024 << (sb)->fragmentsize_offset)

/**
 * A block group is a group of inodes and data blocks.
 *
 * Field descriptions from <http://wiki.osdev.org/Ext2#Block_Group_Descriptor>
 */
struct ext2_block_group {
    uint32_t block_usage_map_bn;   // Block address of block usage bitmap
    uint32_t inode_usage_map_bn;   // Block address of inode usage bitmap
    uint32_t inode_table_bn;       // Starting block address of inode table
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
struct ext2_inode *ext2_traverse_path(struct ext2_disk *disk, struct ext2_inode *cwd, const char *file_path, char **last_segment, uint32_t *container_inode);
struct ext2_inode *ext2_get_inode(struct ext2_disk *disk, uint32_t block_addr, uint32_t inode_addr);
struct ext2_directory_entry *ext2_read_entry_from_directory(struct ext2_disk *disk, struct ext2_inode *cwd, const char *name);
struct ext2_directory_entry *ext2_create_entry(struct ext2_disk *disk, struct ext2_directory_entry *entry, uint32_t inode_addr, struct ext2_inode *inode, char *name);
struct ext2_inode *ext2_create_inode(struct ext2_disk *disk, struct ext2_inode *cwd, const char *last_segment, uint32_t *inode_addr);
int ext2_write_data(struct ext2_disk *disk, struct ext2_inode *file, FILE *source_file);
int ext2_write_directory_data(struct ext2_disk *disk, uint32_t containing_inode_addr, struct ext2_directory_entry *entry);
void ext2_set_inode_in_use(struct ext2_disk *disk, uint32_t inode_addr);
void ext2_free_inode(struct ext2_disk *disk, uint32_t inode_addr);
