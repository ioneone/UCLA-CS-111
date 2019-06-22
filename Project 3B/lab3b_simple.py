#!/usr/bin/python
# Name  : Junhong Wang
# ID    : 504941113
# Email : oneone@g.ucla.edu

from collections import defaultdict
import sys
import csv

EXT2_NDIR_BLOCKS = 12
EXT2_IND_BLOCK = EXT2_NDIR_BLOCKS
EXT2_DIND_BLOCK = EXT2_IND_BLOCK + 1
EXT2_TIND_BLOCK = EXT2_DIND_BLOCK + 1
EXT2_N_BLOCKS = EXT2_TIND_BLOCK + 1
NUM_BLOCK_NUMBER_IN_A_BLOCK = 256

status = 0

class SuperBlock:
	def __init__(self, num_blocks, num_inodes, block_size, inode_size, num_blocks_per_group, num_inodes_per_group, first_non_reserved_inode):
		self.num_blocks = num_blocks                             # total number of blocks (decimal)
		self.num_inodes = num_inodes                             # total number of i-nodes (decimal)
		self.block_size = block_size                             # block size (in bytes, decimal)
		self.inode_size = inode_size                             # i-node size (in bytes, decimal)
		self.num_blocks_per_group = num_blocks_per_group         # blocks per group (decimal) 
		self.num_inodes_per_group = num_inodes_per_group         # i-nodes per group (decimal)
		self.first_non_reserved_inode = first_non_reserved_inode # first non-reserved i-node (decimal)

class Group:
	def __init__(self, group_number, num_blocks, num_inodes, num_free_blocks, num_free_inodes, block_num_of_free_block_bitmap, block_num_of_free_inode_bitmap, block_num_of_first_block_of_inode_table):
		self.group_number = group_number                                                       # group number (decimal, starting from zero)
		self.num_blocks = num_blocks                                                           # total number of blocks in this group (decimal)
		self.num_inodes = num_inodes                                                           # total number of i-nodes in this group (decimal)
		self.num_free_blocks = num_free_blocks                                                 # number of free blocks (decimal)
		self.num_free_inodes = num_free_inodes                                                 # number of free i-nodes (decimal)
		self.block_num_of_free_block_bitmap = block_num_of_free_block_bitmap                   # block number of free block bitmap for this group (decimal)
		self.block_num_of_free_inode_bitmap = block_num_of_free_inode_bitmap                   # block number of free i-node bitmap for this group (decimal)
		self.block_num_of_first_block_of_inode_table = block_num_of_first_block_of_inode_table # block number of first block of i-nodes in this group (decimal)

class BFree:
	def __init__(self, free_entry_index):
		self.free_entry_index = free_entry_index # number of the free entry (decimal)

class IFree:
	def __init__(self, free_entry_index):
		self.free_entry_index = free_entry_index # number of the free entry (decimal)

class INode:
	def __init__(self, inode_number, file_type, mode, owner_id, group_id, link_count, creation_timestamp, modification_timestamp, access_timestamp, file_size, num_blocks_of_disk_space, block_numbers):
		self.inode_number = inode_number                                 # inode number (decimal)
		self.file_type = file_type                                       # file type ('f' for file, 'd' for directory, 's' for symbolic link, '?" for anything else)
		self.mode = mode                                                 # mode (low order 12-bits, octal ... suggested format "%o")
		self.owner_id = owner_id                                         # owner (decimal)
		self.group_id = group_id                                         # group (decimal)
		self.link_count = link_count                                     # link count (decimal)
		self.creation_timestamp = creation_timestamp                     # time of last I-node change (mm/dd/yy hh:mm:ss, GMT)
		self.modification_timestamp = modification_timestamp             # modification time (mm/dd/yy hh:mm:ss, GMT)
		self.access_timestamp = access_timestamp                         # time of last access (mm/dd/yy hh:mm:ss, GMT)
		self.file_size = file_size                                       # file size (decimal)
		self.num_blocks_of_disk_space = num_blocks_of_disk_space         # number of (512 byte) blocks of disk space (decimal) taken up by this file
		self.direct_block_numbers = block_numbers[:EXT2_NDIR_BLOCKS]     # the next fifteen fields are block addresses
		self.indirect_block_numbers = block_numbers[EXT2_NDIR_BLOCKS:EXT2_N_BLOCKS]


class DirEnt:
	def __init__(self, parent_inode_number, logical_byte_offset, inode_number_of_file, entry_length, name_length, name):
		self.parent_inode_number = parent_inode_number   # parent inode number (decimal) ... the I-node number of the directory that contains this entry
		self.logical_byte_offset = logical_byte_offset   # logical byte offset (decimal) of this entry within the directory
		self.inode_number_of_file = inode_number_of_file # inode number of the referenced file (decimal)
		self.entry_length = entry_length                 # entry length (decimal)
		self.name_length = name_length                   # name length (decimal)
		self.name = name                                 # name (string, surrounded by single-quotes). 

class Indirect:
	def __init__(self, inode_number, level, logical_block_offset, indirect_block_number, block_number):
		self.inode_number = inode_number                   # I-node number of the owning file (decimal)
		self.level = level                                 # (decimal) level of indirection for the block being scanned 
		self.logical_block_offset = logical_block_offset   # logical block offset (decimal) represented by the referenced block. 
		self.indirect_block_number = indirect_block_number # block number of the (1, 2, 3) indirect block being scanned (decimal)
		self.block_number = block_number                   # block number of the referenced block (decimal)

def get_usage():
	return "\
Usage: ./lab3b [FILE SYSTEM SUMMARY CSV]\n\
analyze a file system summary and report on all discovered inconsistencies.\n";

def handle_unrecognized_arguments():
	global status
	sys.stderr.write('Unrecognized arguments detected\n')
	sys.stderr.write(get_usage())
	status = 1

def handle_IOError():
	global status
	sys.stderr.write('Error opening the file\n')
	status = 1

def handle_IndexError():
	global status
	sys.stderr.write('Incorrect file format\n')
	status = 1

def handle_generic_error():
	global status
	sys.stderr.write('Something went wrong\n')
	status = 1

def main():
	if len(sys.argv) != 2:
		handle_unrecognized_arguments()

	global status

	try:
		with open(sys.argv[1]) as csv_file:
			csv_reader = csv.reader(csv_file, delimiter=',')

			super_block   = None
			group         = None
			bfree_list    = []
			ifree_list    = []
			inode_list    = []
			dirent_list   = []
			indirect_list = []

			referenced_blocks = defaultdict(list)

			for row in csv_reader:
				if   row[0] == 'SUPERBLOCK': super_block    = SuperBlock(int(row[1]), int(row[2]), int(row[3]), int(row[4]), int(row[5]), int(row[6]), int(row[7]))
				elif row[0] == 'GROUP':      group          = Group(int(row[1]), int(row[2]), int(row[3]), int(row[4]), int(row[5]), int(row[6]), int(row[7]), int(row[8]))
				elif row[0] == 'BFREE':      bfree_list    += [BFree(int(row[1]))]
				elif row[0] == 'IFREE':      ifree_list    += [IFree(int(row[1]))]
				elif row[0] == 'INODE':      inode_list    += [INode(int(row[1]), row[2], row[3], int(row[4]), int(row[5]), int(row[6]), row[7], row[8], row[9], int(row[10]), int(row[11]), list(map(int, row[12:])))]
				elif row[0] == 'DIRENT':     dirent_list   += [DirEnt(int(row[1]), int(row[2]), int(row[3]), int(row[4]), int(row[5]), row[6])]
				elif row[0] == 'INDIRECT':   indirect_list += [Indirect(int(row[1]), int(row[2]), int(row[3]), int(row[4]), int(row[5]))]

			# generate block consistency audits
			block_num_of_first_block_of_inodes = group.block_num_of_first_block_of_inode_table + super_block.inode_size * group.num_inodes / super_block.block_size

			## check invalid/reserved
			for inode in inode_list:
				for i in range(len(inode.direct_block_numbers)):

					direct_block_number = inode.direct_block_numbers[i]

					if direct_block_number == 0: continue

					if direct_block_number < 0 or direct_block_number >= super_block.num_blocks:
						
						print('INVALID BLOCK {} IN INODE {} AT OFFSET {}'.format(direct_block_number, inode.inode_number, i))
						status = 2
					elif direct_block_number < block_num_of_first_block_of_inodes:
						print('RESERVED BLOCK {} IN INODE {} AT OFFSET {}'.format(direct_block_number, inode.inode_number, i))
						status = 2
					else:
						referenced_blocks[direct_block_number].append(('BLOCK', direct_block_number, inode.inode_number, i))


				for i in range(len(inode.indirect_block_numbers)):

					indirect_block_number = inode.indirect_block_numbers[i]

					if indirect_block_number == 0: continue

					level = None
					if   (i == 0): level = 'INDIRECT'
					elif (i == 1): level = 'DOUBLE INDIRECT'
					elif (i == 2): level = 'TRIPLE INDIRECT'

					logical_block_offset = None
					if   (i == 0): logical_block_offset = EXT2_NDIR_BLOCKS
					elif (i == 1): logical_block_offset = EXT2_NDIR_BLOCKS + NUM_BLOCK_NUMBER_IN_A_BLOCK
					elif (i == 2): logical_block_offset = EXT2_NDIR_BLOCKS + (1 + NUM_BLOCK_NUMBER_IN_A_BLOCK) * NUM_BLOCK_NUMBER_IN_A_BLOCK

					if indirect_block_number < 0 or indirect_block_number >= super_block.num_blocks:
						print('INVALID {} BLOCK {} IN INODE {} AT OFFSET {}'.format(level, indirect_block_number, inode.inode_number, logical_block_offset))
						status = 2
					elif indirect_block_number < block_num_of_first_block_of_inodes:
						print('RESERVED {} BLOCK {} IN INODE {} AT OFFSET {}'.format(level, indirect_block_number, inode.inode_number, logical_block_offset))
						status = 2
					else:
						referenced_blocks[indirect_block_number].append((level + ' BLOCK', indirect_block_number, inode.inode_number, logical_block_offset))

			for indirect in indirect_list:

				if indirect.block_number == 0: continue

				level = None
				if   (indirect.level == 1): level = 'INDIRECT'
				elif (indirect.level == 2): level = 'DOUBLE INDIRECT'
				elif (indirect.level == 3): level = 'TRIPLE INDIRECT'

				logical_block_offset = None
				if   (indirect.level == 1): logical_block_offset = EXT2_NDIR_BLOCKS
				elif (indirect.level == 2): logical_block_offset = EXT2_NDIR_BLOCKS + NUM_BLOCK_NUMBER_IN_A_BLOCK
				elif (indirect.level == 3): logical_block_offset = EXT2_NDIR_BLOCKS + (1 + NUM_BLOCK_NUMBER_IN_A_BLOCK) * NUM_BLOCK_NUMBER_IN_A_BLOCK

				if indirect.block_number < 0 or indirect.block_number >= super_block.num_blocks:
					print('INVALID {} BLOCK {} IN INODE {} AT OFFSET {}'.format(level, indirect.block_number, indirect.inode_number, logical_block_offset))
					status = 2
				elif indirect.block_number < block_num_of_first_block_of_inodes:
					print('RESERVED {} BLOCK {} IN INODE {} AT OFFSET {}'.format(level, indirect.block_number, indirect.inode_number, logical_block_offset))
					status = 2
				else:
					referenced_blocks[indirect.block_number].append((level + ' BLOCK', indirect.block_number, indirect.inode_number, logical_block_offset))

			## check unreferenced
			free_block_numbers = list(map(lambda bfree: bfree.free_entry_index, bfree_list))

			for block_number in range(block_num_of_first_block_of_inodes, super_block.num_blocks):
				if block_number not in referenced_blocks and block_number not in free_block_numbers:
					print('UNREFERENCED BLOCK {}'.format(block_number))
					status = 2

			## check allocated
			for block_number in range(block_num_of_first_block_of_inodes, super_block.num_blocks):
				if block_number in referenced_blocks and block_number in free_block_numbers:
					print('ALLOCATED BLOCK {} ON FREELIST'.format(block_number))
					status = 2

			## check duplicate
			for block_number in range(block_num_of_first_block_of_inodes, super_block.num_blocks):
				if block_number not in referenced_blocks: continue
				li = referenced_blocks[block_number]
				if len(li) == 1: continue
				for data in li:
					print('DUPLICATE {} {} IN INODE {} AT OFFSET {}'.format(data[0], data[1], data[2], data[3]))
					status = 2

			# generate inode allocation audits
			free_inode_numbers = list(map(lambda ifree: ifree.free_entry_index, ifree_list))
			allocated_inode_numbers = list(map(lambda inode: inode.inode_number, inode_list))

			## check allocated
			for inode_number in allocated_inode_numbers:
				if inode_number in free_inode_numbers:
					print('ALLOCATED INODE {} ON FREELIST'.format(inode_number))
					status = 2

			## check unallocated
			for inode_number in range(super_block.first_non_reserved_inode, super_block.num_inodes + 1):
				if inode_number not in allocated_inode_numbers and inode_number not in free_inode_numbers:
					print('UNALLOCATED INODE {} NOT ON FREELIST'.format(inode_number))
					status = 2


			# generate directory consistency audits
			link_count_dict = defaultdict(int)
			parent_dict = {}
			
			## check directory entries
			for dirent in dirent_list: 
				if dirent.inode_number_of_file < 1 or dirent.inode_number_of_file > super_block.num_inodes:
					print('DIRECTORY INODE {} NAME {} INVALID INODE {}'.format(dirent.parent_inode_number, dirent.name, dirent.inode_number_of_file))
					status = 2
				elif dirent.inode_number_of_file not in allocated_inode_numbers:
					print('DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}'.format(dirent.parent_inode_number, dirent.name, dirent.inode_number_of_file))
					status = 2
				else:
					link_count_dict[dirent.inode_number_of_file] += 1
					if dirent.inode_number_of_file not in parent_dict:
						parent_dict[dirent.inode_number_of_file] = dirent.parent_inode_number

			## check inode
			for inode in inode_list:
				if inode.link_count != link_count_dict[inode.inode_number]:
					print('INODE {} HAS {} LINKS BUT LINKCOUNT IS {}'.format(inode.inode_number, link_count_dict[inode.inode_number], inode.link_count))
					status = 2

			## check special directory entries . and ..
			for dirent in dirent_list:
				if dirent.name == "'.'" and dirent.parent_inode_number != dirent.inode_number_of_file:
					print('DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}'.format(dirent.parent_inode_number, dirent.name, dirent.inode_number_of_file, dirent.parent_inode_number))
					status = 2

			for dirent in dirent_list:
				if dirent.name == "'..'" and parent_dict[dirent.parent_inode_number] != dirent.inode_number_of_file:
					print('DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}'.format(dirent.parent_inode_number, dirent.name, dirent.inode_number_of_file, dirent.parent_inode_number))
					status = 2


	except IOError:
		handle_IOError()
	except IndexError:
		handle_IndexError()
	except:
		handle_generic_error()


	sys.exit(status)


if __name__ == '__main__':
	main()






