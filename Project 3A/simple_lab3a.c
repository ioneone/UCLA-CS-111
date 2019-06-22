// Name  : Junhong Wang
// ID    : 504941113
// Email : oneone@g.ucla.edu

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "ext2_fs.h"


// number of bits in byte
#define BYTE 8

// An Ext2 file systems starts with a superblock 
// located at byte offset 1024 from the start of the volume.
#define SUPER_BLOCK_OFFSET 1024

// block group descriptor table has block offset 2
#define GROUP_DESCRIPTOR_TABLE_OFFSET 2048

// file types
#define FILE          0x8000
#define DIRECTORY     0x4000
#define SYMBOLIC_LINK 0xA000

// file abbreviation
#define FILE_ABBREVIATION          'f'
#define DIRECTORY_ABBREVIATION     'd'
#define SYMBOLIC_LINK_ABBREVIATION 's'

// inode file type extraction
#define FILE_TYPE_REGION 0xF000

// inode mode extraction
#define MODE_REGION 0x0FFF

#define TIMESTAMP_SIZE 100

// Symbolic links are a little more complicated. 
// If the file length is less than the size of the block pointers (60 bytes) 
// the file will contain zero data blocks, 
// and the name is stored in the space normally occupied by the block pointers. 
#define MIN_SYMBOLIC_FILE_LEGTH 60

// free entry type
#define FREE_BLOCK_ENTRY 'B'
#define FREE_INODE_ENTRY 'I'

// file system image file descriptor
int fsi_fd;

char* get_usage() {
	return 
		"\
Usage: ./lab3a [FILE SYSTEM IMAGE]\n\
Analyzes the provided file system image and produces (to standard out)\n\
CSV summaries of what it finds.\n\
		";
}

void handle_unrecognized_arguments() {
	fprintf(stderr, "Unrecognized arguments detected\n%s\n", get_usage());
	exit(1);
}

int safe_open(char *pathname, int flags) {
	int fd = open(pathname, flags);
	if (fd == -1) {
		fprintf(stderr, "Failed to open a file: %s\n", strerror(errno));
		exit(1);
	}
	return fd;

}

int safe_pread(int fd, void *buf, int count, int offset) {
	int ret = pread(fd, buf, count, offset);
	if (ret == -1) {
		fprintf(stderr, "Failed to pread a file descriptor: %s\n", strerror(errno));
		exit(1);
	}
	return ret;
}

int is_nth_bit_set(unsigned char target, int n) {
	// check if n-th bit is 1
	return target & (1 << (n - 1));
}

int calculate_index(int byte_index, int bit_index) {
	// free blocks and inodes are indexed as 1, 2, 3, ...
	return BYTE * (byte_index - 1) + bit_index; 
}

int calculate_block_size(struct ext2_super_block *super_block) {
	return EXT2_MIN_BLOCK_SIZE << super_block->s_log_block_size;
}

int calculate_num_block_number_in_a_block(struct ext2_super_block *super_block) {
	return calculate_block_size(super_block) / sizeof(int);
}

int is_inode_free(struct ext2_inode *inode) {
	return inode->i_mode == 0 || inode->i_links_count == 0;
}

int is_inode_file_type_file(struct ext2_inode *inode) {
	return (inode->i_mode & FILE_TYPE_REGION) == FILE;
}

int is_inode_file_type_directory(struct ext2_inode *inode) {
	return (inode->i_mode & FILE_TYPE_REGION) == DIRECTORY;
}

int is_inode_file_type_symbolic_link(struct ext2_inode *inode) {
	return (inode->i_mode & FILE_TYPE_REGION) == SYMBOLIC_LINK;
}

char extract_inode_file_type(struct ext2_inode *inode) {

	if (is_inode_file_type_file(inode))          return FILE_ABBREVIATION;
	if (is_inode_file_type_directory(inode))     return DIRECTORY_ABBREVIATION;
	if (is_inode_file_type_symbolic_link(inode)) return SYMBOLIC_LINK_ABBREVIATION;
	
	return '?';
}

int extract_inode_mode(struct ext2_inode *inode) {
	return inode->i_mode & MODE_REGION;
}

void extract_timestamp(char *timestamp, unsigned int time) {
	time_t rawtime = time;
	struct tm *info = gmtime(&rawtime);
	strftime(timestamp, TIMESTAMP_SIZE, "%m/%d/%y %H:%M:%S", info);
}

void extract_inode_creation_timestamp(char *timestamp, struct ext2_inode *inode) {
	extract_timestamp(timestamp, inode->i_ctime);
}

void extract_inode_last_modification_timestamp(char *timestamp, struct ext2_inode *inode) {
	extract_timestamp(timestamp, inode->i_mtime);
}

void extract_inode_last_access_timestamp(char *timestamp, struct ext2_inode *inode) {
	extract_timestamp(timestamp, inode->i_atime);
}

int is_next_fifteen_fields_needed(struct ext2_inode *inode) {
	return extract_inode_file_type(inode) != SYMBOLIC_LINK_ABBREVIATION || inode->i_size >= MIN_SYMBOLIC_FILE_LEGTH;
}

void initialize_super_block(struct ext2_super_block *super_block) {
	safe_pread(
		fsi_fd, 
		super_block, 
		sizeof(struct ext2_super_block), 
		SUPER_BLOCK_OFFSET
	);
}

void generate_super_block_summary(struct ext2_super_block *super_block) {
	fprintf(
		stdout, 
		"%s,%d,%d,%d,%d,%d,%d,%d\n", 
		"SUPERBLOCK", 
		super_block->s_blocks_count,         // total number of blocks (decimal)
		super_block->s_inodes_count,         // total number of i-nodes (decimal)
		calculate_block_size(super_block),   // block size (in bytes, decimal)
		super_block->s_inode_size,           // i-node size (in bytes, decimal)
		super_block->s_blocks_per_group,     // blocks per group (decimal) 
		super_block->s_inodes_per_group,     // i-nodes per group (decimal)
		super_block->s_first_ino             // first non-reserved i-node (decimal)
	);
}

void initialize_group_descriptor(struct ext2_group_desc *group_desc) {
	safe_pread(
		fsi_fd, 
		group_desc, 
		sizeof(struct ext2_group_desc), 
		GROUP_DESCRIPTOR_TABLE_OFFSET
	);
}

void generate_group_summary(struct ext2_super_block *super_block, struct ext2_group_desc *group_desc) {
	fprintf(
		stdout,
		"%s,%d,%d,%d,%d,%d,%d,%d,%d\n",
		"GROUP",
		0,                                // group number (decimal, starting from zero)
		super_block->s_blocks_count,      // total number of blocks in this group (decimal)
		super_block->s_inodes_count,      // total number of i-nodes in this group (decimal)
		group_desc->bg_free_blocks_count, // number of free blocks (decimal)
		group_desc->bg_free_inodes_count, // number of free i-nodes (decimal)
		group_desc->bg_block_bitmap,      // block number of free block bitmap for this group (decimal)
		group_desc->bg_inode_bitmap,      // block number of free i-node bitmap for this group (decimal)
		group_desc->bg_inode_table        // block number of first block of i-nodes in this group (decimal)
	);
}

void prepare_for_free_entry_summary(char entry_type, struct ext2_group_desc *group_desc, int *block_number_of_bitmap_location, char *key) {
	if (entry_type == FREE_BLOCK_ENTRY) {
		*block_number_of_bitmap_location = group_desc->bg_block_bitmap;
		strcpy(key, "BFREE");
	}
	else if (entry_type == FREE_INODE_ENTRY) {
		*block_number_of_bitmap_location = group_desc->bg_inode_bitmap;
		strcpy(key, "IFREE");
	}
}

void produce_one_free_entry_info(char *key, int entry_index) {
	fprintf(
		stdout, 
		"%s,%d\n",
		key,
		entry_index // number of the free entry (decimal)
	);
}

void produce_free_entry_summary(struct ext2_super_block *super_block, int block_number_of_bitmap_location, char *key) {
	int i, j;
	unsigned char byte;

	// loop through each byte of the bit map
	for (i = 0; i < calculate_block_size(super_block); i++) {

		safe_pread(
			fsi_fd,
			&byte, 
			sizeof(unsigned char), 
			block_number_of_bitmap_location * calculate_block_size(super_block) + i
		);

		// loop through each bit of the byte
		for (j = 0; j < BYTE; j++)
			if (!is_nth_bit_set(byte, j + 1)) produce_one_free_entry_info(key, calculate_index(i + 1, j + 1));

	}
}

void generate_free_entry_summary(struct ext2_super_block *super_block, struct ext2_group_desc *group_desc, char entry_type) {
	
	int block_number_of_bitmap_location;
	char key[10];

	prepare_for_free_entry_summary(entry_type, group_desc, &block_number_of_bitmap_location, key);
	produce_free_entry_summary(super_block, block_number_of_bitmap_location, key);
	
}

void generate_free_block_entry_summary(struct ext2_super_block *super_block, struct ext2_group_desc *group_desc) {
	generate_free_entry_summary(super_block, group_desc, FREE_BLOCK_ENTRY);
}

void generate_free_inode_entry_summary(struct ext2_super_block *super_block, struct ext2_group_desc *group_desc) {
	generate_free_entry_summary(super_block, group_desc, FREE_INODE_ENTRY);
}

void produce_one_inode_info(int inode_number, struct ext2_inode *inode) {
	int i;

	char file_type = extract_inode_file_type(inode);
	char creation_timestamp[TIMESTAMP_SIZE];
	char last_modification_timestamp[TIMESTAMP_SIZE];
	char last_access_timestamp[TIMESTAMP_SIZE];

	extract_inode_creation_timestamp(creation_timestamp, inode);
	extract_inode_last_modification_timestamp(last_modification_timestamp, inode);
	extract_inode_last_access_timestamp(last_access_timestamp, inode);

	fprintf(
		stdout,
		"%s,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d",
		"INODE",
		inode_number,                // inode number (decimal)
		file_type,                   // file type ('f' for file, 'd' for directory, 's' for symbolic link, '?" for anything else)
		extract_inode_mode(inode),   // mode (low order 12-bits, octal ... suggested format "%o")
		inode->i_uid,                // owner (decimal)
		inode->i_gid,                // group (decimal)
		inode->i_links_count,        // link count (decimal)
		creation_timestamp,          // time of last I-node change (mm/dd/yy hh:mm:ss, GMT)
		last_modification_timestamp, // modification time (mm/dd/yy hh:mm:ss, GMT)
		last_access_timestamp,       // time of last access (mm/dd/yy hh:mm:ss, GMT)
		inode->i_size,               // file size (decimal)
		inode->i_blocks              // number of (512 byte) blocks of disk space (decimal) taken up by this file
	);


	if (is_next_fifteen_fields_needed(inode)) {
		for (i = 0; i < EXT2_N_BLOCKS; i++)
			fprintf(stdout, ",%d", inode->i_block[i]); // the next fifteen fields are block addresses
	}

	fprintf(stdout, "\n");
}

void generate_inode_summary(struct ext2_super_block *super_block, struct ext2_group_desc *group_desc) {
	int i;
	struct ext2_inode inode;

	// loop through each inode
	for (i = 0; i < (int) super_block->s_inodes_count; i++) {

		safe_pread(
			fsi_fd, 
			&inode, 
			sizeof(struct ext2_inode), 
			group_desc->bg_inode_table * calculate_block_size(super_block) + i * sizeof(struct ext2_inode)
		);

		if (is_inode_free(&inode)) continue;

		produce_one_inode_info(i + 1, &inode);
		
	}

}

void produce_one_directory_entry_info(struct ext2_dir_entry *dir_entry, int inode_number, int logical_offset) {
	fprintf(
		stdout, 
		"%s,%d,%d,%d,%d,%d,'%s'\n", 
		"DIRENT",
		inode_number,        // parent inode number (decimal) ... the I-node number of the directory that contains this entry
		logical_offset,      // logical byte offset (decimal) of this entry within the directory
		dir_entry->inode,    // inode number of the referenced file (decimal)
		dir_entry->rec_len,  // entry length (decimal)
		dir_entry->name_len, // name length (decimal)
		dir_entry->name      // name (string, surrounded by single-quotes). 
		                     // Don't worry about escaping, we promise there will be no single-quotes or commas in any of the file names.
	);
}

void generate_directory_entry_summary(struct ext2_super_block *super_block, struct ext2_group_desc *group_desc) {
	int i, j, k;
	struct ext2_inode inode;
	struct ext2_dir_entry dir_entry;

	for (i = 0; i < (int) super_block->s_inodes_count; i++) {

		safe_pread(
			fsi_fd, 
			&inode, 
			sizeof(struct ext2_inode), 
			group_desc->bg_inode_table * calculate_block_size(super_block) + i * sizeof(struct ext2_inode)
		);

		if (is_inode_free(&inode)) continue;

		if (extract_inode_file_type(&inode) != DIRECTORY_ABBREVIATION) continue;
		
		// loop through each block reference
		for (j = 0; j < EXT2_N_BLOCKS; j++) {

			if (inode.i_block[j] == 0) continue; // invalid block reference

			// loop through each entry in the block
			for (k = 0; k < calculate_block_size(super_block); k += dir_entry.rec_len) {

				safe_pread(
					fsi_fd, 
					&dir_entry, 
					sizeof(struct ext2_dir_entry), 
					inode.i_block[j] * calculate_block_size(super_block) + k
				);

				if (dir_entry.inode == 0) continue; // invalid inode number

				produce_one_directory_entry_info(&dir_entry, i + 1, k);

				
			}

			
		}
	}
}

int generate_indirect_block_reference_summary_helper(struct ext2_super_block *super_block, int indirect_block_number, int inode_number, int offset, int index, int level) {
	int block_number; // block number is 32 bits

	safe_pread(
		fsi_fd, 
		&block_number, 
		sizeof(int), 
		indirect_block_number * calculate_block_size(super_block) + index * sizeof(int)
	);

	if (block_number == 0) return block_number; // invalid block reference

	fprintf(
		stdout, 
		"%s,%d,%d,%d,%d,%d\n",
		"INDIRECT",
		inode_number,                     // I-node number of the owning file (decimal)
		level,                            // (decimal) level of indirection for the block being scanned 
		                                  // ... 1 for single indirect, 2 for double indirect, 3 for triple
		offset,                           // logical block offset (decimal) represented by the referenced block. 
		                                  // If the referenced block is a data block, this is the logical block offset of that block within the file. 
		                                  // If the referenced block is a single- or double-indirect block, 
		                                  // this is the same as the logical offset of the first data block to which it refers.
		indirect_block_number,            // block number of the (1, 2, 3) indirect block being scanned (decimal) . . . not the highest level block (in the recursive scan), 
		                                  // but the lower level block that contains the block reference reported by this entry.
		block_number                      // block number of the referenced block (decimal)
	);

	return block_number;
}

void generate_single_indirect_block_reference_summary(struct ext2_super_block *super_block, struct ext2_inode *inode, int inode_number) {
	int i;

	for (i = 0; i < calculate_num_block_number_in_a_block(super_block); i++) {

		generate_indirect_block_reference_summary_helper(super_block, inode->i_block[EXT2_IND_BLOCK], inode_number, EXT2_NDIR_BLOCKS + i, i, 1);
		// safe_pread(
		// 	fsi_fd, 
		// 	&block_number, 
		// 	sizeof(int), 
		// 	inode->i_block[EXT2_IND_BLOCK] * calculate_block_size(super_block) + i * sizeof(int)
		// );

		// if (block_number == 0) continue; // invalid block reference

		// fprintf(
		// 	stdout, 
		// 	"%s,%d,%d,%d,%d,%d\n",
		// 	"INDIRECT",
		// 	inode_number,                   // I-node number of the owning file (decimal)
		// 	1,                              // (decimal) level of indirection for the block being scanned 
		// 	                                // ... 1 for single indirect, 2 for double indirect, 3 for triple
		// 	EXT2_NDIR_BLOCKS + i,           // logical block offset (decimal) represented by the referenced block. 
		// 	                                // If the referenced block is a data block, this is the logical block offset of that block within the file. 
		// 	                                // If the referenced block is a single- or double-indirect block, 
		// 	                                // this is the same as the logical offset of the first data block to which it refers.
		// 	inode->i_block[EXT2_IND_BLOCK], // block number of the (1, 2, 3) indirect block being scanned (decimal) . . . not the highest level block (in the recursive scan), 
		// 	                                // but the lower level block that contains the block reference reported by this entry.
		// 	block_number                    // block number of the referenced block (decimal)
		// );

	}
}

void generate_double_indirect_block_reference_summary(struct ext2_super_block *super_block, struct ext2_inode *inode, int inode_number) {
	int i, j;
	int indirect_block_number;

	for (i = 0; i < calculate_num_block_number_in_a_block(super_block); i++) {

		indirect_block_number = generate_indirect_block_reference_summary_helper(super_block, inode->i_block[EXT2_DIND_BLOCK], inode_number, EXT2_NDIR_BLOCKS + (i + 1) * calculate_num_block_number_in_a_block(super_block), i, 2);
		// safe_pread(
		// 	fsi_fd, 
		// 	&indirect_block_number, 
		// 	sizeof(int), 
		// 	inode->i_block[EXT2_DIND_BLOCK] * calculate_block_size(super_block) + i * sizeof(int)
		// );

		if (indirect_block_number == 0) continue;

		// fprintf(
		// 	stdout, 
		// 	"%s,%d,%d,%d,%d,%d\n",
		// 	"INDIRECT",
		// 	inode_number,                                                                            // I-node number of the owning file (decimal)
		// 	2,                                                                                // (decimal) level of indirection for the block being scanned 
		// 	                                                                                  // ... 1 for single indirect, 2 for double indirect, 3 for triple
		// 	EXT2_NDIR_BLOCKS + (i + 1) * calculate_num_block_number_in_a_block(super_block), // logical block offset (decimal) represented by the referenced block. 
		// 	                                                                                  // If the referenced block is a data block, this is the logical block offset of that block within the file. 
		// 	                                                                                  // If the referenced block is a single- or double-indirect block, 
		// 	                                                                                  // this is the same as the logical offset of the first data block to which it refers.
		// 	inode->i_block[EXT2_DIND_BLOCK],                                                   // block number of the (1, 2, 3) indirect block being scanned (decimal) . . . not the highest level block (in the recursive scan), 
		// 	                                                                                  // but the lower level block that contains the block reference reported by this entry.
		// 	indirect_block_number                                                             // block number of the referenced block (decimal)
		// );


		for (j = 0; j < calculate_num_block_number_in_a_block(super_block); j++) {

			generate_indirect_block_reference_summary_helper(super_block, indirect_block_number, inode_number, EXT2_NDIR_BLOCKS + (i + 1) * calculate_num_block_number_in_a_block(super_block) + j, j, 1);

			// safe_pread(
			// 	fsi_fd, 
			// 	&block_number, 
			// 	sizeof(int), 
			// 	indirect_block_number * calculate_block_size(super_block) + j * sizeof(int)
			// );

			// if (block_number == 0) continue;

			// fprintf(
			// 	stdout, 
			// 	"%s,%d,%d,%d,%d,%d\n",
			// 	"INDIRECT",
			// 	inode_number,                                                                                // I-node number of the owning file (decimal)
			// 	1,                                                                                    // (decimal) level of indirection for the block being scanned 
			// 	                                                                                      // ... 1 for single indirect, 2 for double indirect, 3 for triple
			// 	EXT2_NDIR_BLOCKS + (i + 1) * calculate_num_block_number_in_a_block(super_block) + j, // logical block offset (decimal) represented by the referenced block. 
			//                                                                                           // If the referenced block is a data block, this is the logical block offset of that block within the file. 
			//                                                                                           // If the referenced block is a single- or double-indirect block, 
			//                                                                                           // this is the same as the logical offset of the first data block to which it refers.
			// 	indirect_block_number,                                                                // block number of the (1, 2, 3) indirect block being scanned (decimal) . . . not the highest level block (in the recursive scan), 
			//                                                                                           // but the lower level block that contains the block reference reported by this entry.
			// 	block_number                                                                          // block number of the referenced block (decimal)
			// );

		}

	}

}

void generate_triple_indirect_block_reference_summary(struct ext2_super_block *super_block, struct ext2_inode *inode, int inode_number) {
	int i, j, k;
	int double_indirect_block_number;
	int indirect_block_number;

	for (i = 0; i < calculate_num_block_number_in_a_block(super_block); i++) {

		double_indirect_block_number = generate_indirect_block_reference_summary_helper(super_block, inode->i_block[EXT2_TIND_BLOCK], inode_number, EXT2_NDIR_BLOCKS + ((i + 1) * calculate_num_block_number_in_a_block(super_block) + 1) * calculate_num_block_number_in_a_block(super_block), i, 3);

		// safe_pread(
		// 	fsi_fd, 
		// 	&double_indirect_block_number, 
		// 	sizeof(int), 
		// 	inode->i_block[EXT2_TIND_BLOCK] * calculate_block_size(super_block) + i * sizeof(int)
		// );

		if (double_indirect_block_number == 0) continue;

		// fprintf(
		// 	stdout, 
		// 	"%s,%d,%d,%d,%d,%d\n",
		// 	"INDIRECT",
		// 	inode_number,                                                                                                                                        // I-node number of the owning file (decimal)
		// 	3,                                                                                                                                            // (decimal) level of indirection for the block being scanned 
		// 	                                                                                                                                              // ... 1 for single indirect, 2 for double indirect, 3 for triple
		// 	EXT2_NDIR_BLOCKS + ((i + 1) * calculate_num_block_number_in_a_block(super_block) + 1) * calculate_num_block_number_in_a_block(super_block), // logical block offset (decimal) represented by the referenced block. 
		// 	                                                                                                                                              // If the referenced block is a data block, this is the logical block offset of that block within the file. 
		// 	                                                                                                                                              // If the referenced block is a single- or double-indirect block, 
		// 	                                                                                                                                              // this is the same as the logical offset of the first data block to which it refers.
		// 	inode->i_block[EXT2_TIND_BLOCK],                                                                                                               // block number of the (1, 2, 3) indirect block being scanned (decimal) . . . not the highest level block (in the recursive scan), 
		// 	                                                                                                                                              // but the lower level block that contains the block reference reported by this entry.
		// 	double_indirect_block_number                                                                                                                  // block number of the referenced block (decimal)
		// );

		for (j = 0; j < calculate_num_block_number_in_a_block(super_block); j++) {

			indirect_block_number = generate_indirect_block_reference_summary_helper(super_block, double_indirect_block_number, inode_number, EXT2_NDIR_BLOCKS + ((i + 1) * calculate_num_block_number_in_a_block(super_block) + 1 + j) * calculate_num_block_number_in_a_block(super_block), j, 2);

			// safe_pread(
			// 	fsi_fd, 
			// 	&indirect_block_number, 
			// 	sizeof(int), 
			// 	double_indirect_block_number * calculate_block_size(super_block) + j * sizeof(int)
			// );

			if (indirect_block_number == 0) continue;

			// fprintf(
			// 	stdout, 
			// 	"%s,%d,%d,%d,%d,%d\n",
			// 	"INDIRECT",
			// 	inode_number,                                                                            // I-node number of the owning file (decimal)
			// 	2,                                                                                // (decimal) level of indirection for the block being scanned 
			// 	                                                                                  // ... 1 for single indirect, 2 for double indirect, 3 for triple
			// 	EXT2_NDIR_BLOCKS + ((i + 1) * calculate_num_block_number_in_a_block(super_block) + 1 + j) * calculate_num_block_number_in_a_block(super_block), // logical block offset (decimal) represented by the referenced block. 
			// 	                                                                                  // If the referenced block is a data block, this is the logical block offset of that block within the file. 
			// 	                                                                                  // If the referenced block is a single- or double-indirect block, 
			// 	                                                                                  // this is the same as the logical offset of the first data block to which it refers.
			// 	double_indirect_block_number,                                                   // block number of the (1, 2, 3) indirect block being scanned (decimal) . . . not the highest level block (in the recursive scan), 
			// 	                                                                                  // but the lower level block that contains the block reference reported by this entry.
			// 	indirect_block_number                                                             // block number of the referenced block (decimal)
			// );
			
			for (k = 0; k < calculate_num_block_number_in_a_block(super_block); k++) {

				generate_indirect_block_reference_summary_helper(super_block, indirect_block_number, inode_number, EXT2_NDIR_BLOCKS + ((i + 1) * calculate_num_block_number_in_a_block(super_block) + 1 + j) * calculate_num_block_number_in_a_block(super_block) + k, k, 1);

				// safe_pread(
				// 	fsi_fd, 
				// 	&block_number, 
				// 	sizeof(int), 
				// 	indirect_block_number * calculate_block_size(super_block) + k * sizeof(int)
				// );

				// if (block_number == 0) continue;

				// fprintf(
				// 	stdout, 
				// 	"%s,%d,%d,%d,%d,%d\n",
				// 	"INDIRECT",
				// 	inode_number,                                                                                                                                                // I-node number of the owning file (decimal)
				// 	1,                                                                                                                                                    // (decimal) level of indirection for the block being scanned 
				// 	                                                                                                                                                      // ... 1 for single indirect, 2 for double indirect, 3 for triple
				// 	EXT2_NDIR_BLOCKS + ((i + 1) * calculate_num_block_number_in_a_block(super_block) + 1 + j) * calculate_num_block_number_in_a_block(super_block) + k, // logical block offset (decimal) represented by the referenced block. 
				//                                                                                                                                                           // If the referenced block is a data block, this is the logical block offset of that block within the file. 
				//                                                                                                                                                           // If the referenced block is a single- or double-indirect block, 
				//                                                                                                                                                           // this is the same as the logical offset of the first data block to which it refers.
				// 	indirect_block_number,                                                                                                                                // block number of the (1, 2, 3) indirect block being scanned (decimal) . . . not the highest level block (in the recursive scan), 
				//                                                                                                                                                           // but the lower level block that contains the block reference reported by this entry.
				// 	block_number                                                                                                                                          // block number of the referenced block (decimal)
				// );

			}

		}

	}
}

void generate_indirect_block_reference_summary(struct ext2_super_block *super_block, struct ext2_group_desc *group_desc) {
	int i;
	struct ext2_inode inode;
	char file_type;

	for (i = 0; i < (int) super_block->s_inodes_count; i++) {

		safe_pread(
			fsi_fd, 
			&inode, 
			sizeof(struct ext2_inode), 
			group_desc->bg_inode_table * calculate_block_size(super_block) + i * sizeof(struct ext2_inode)
		);

		if (is_inode_free(&inode)) continue;

		file_type = extract_inode_file_type(&inode);
		
		if (file_type != FILE_ABBREVIATION && file_type != DIRECTORY_ABBREVIATION) continue;

		if (inode.i_block[EXT2_IND_BLOCK]  != 0) generate_single_indirect_block_reference_summary(super_block, &inode, i + 1);
		if (inode.i_block[EXT2_DIND_BLOCK] != 0) generate_double_indirect_block_reference_summary(super_block, &inode, i + 1);
		if (inode.i_block[EXT2_TIND_BLOCK] != 0) generate_triple_indirect_block_reference_summary(super_block, &inode, i + 1);

	}
}

void generate_file_system_summaries() {

	struct ext2_super_block super_block;
	struct ext2_group_desc group_desc;
	
	// generate super block summary
	initialize_super_block(&super_block);
	generate_super_block_summary(&super_block);
	
	// generate group summary
	// But, in the images we give you, there will be only a single group.
	initialize_group_descriptor(&group_desc);
	generate_group_summary(&super_block, &group_desc);
	
	// free block entries
	// Each bit represent the current state of a block within that block group, 
	// where 1 means "used" and 0 "free/available".
	generate_free_block_entry_summary(&super_block, &group_desc);

	// free i-nodes entries
	// 1 means "used" and 0 "free/available"
	generate_free_inode_entry_summary(&super_block, &group_desc);

	// i-node summary
	generate_inode_summary(&super_block, &group_desc);

	// generate directory entry summary
	generate_directory_entry_summary(&super_block, &group_desc);

	// generate indirect block reference summary
	generate_indirect_block_reference_summary(&super_block, &group_desc);


}

void init(int argc, char **args) {
	if (argc != 2) handle_unrecognized_arguments();

	// read only to prevent you from accidentally changing the image.
	fsi_fd = safe_open(args[1], O_RDONLY);
}

void run() {
	generate_file_system_summaries();
}

void clean_up() {
	close(fsi_fd);
}

int main(int argc, char **args) {
	init(argc, args);
	run();
	clean_up();
	exit(0);
}