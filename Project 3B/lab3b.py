#!/usr/bin/python
# Name  : Junhong Wang
# ID    : 504941113
# Email : oneone@g.ucla.edu

from collections import defaultdict
import sys
import csv

EXT2_NDIR_BLOCKS            = 12
EXT2_IND_BLOCK              = EXT2_NDIR_BLOCKS
EXT2_DIND_BLOCK             = EXT2_IND_BLOCK + 1
EXT2_TIND_BLOCK             = EXT2_DIND_BLOCK + 1
EXT2_N_BLOCKS               = EXT2_TIND_BLOCK + 1
NUM_BLOCK_NUMBER_IN_A_BLOCK = 256

class SuperBlock:
	def __init__(self, num_blocks, num_inodes, block_size, inode_size, num_blocks_per_group, num_inodes_per_group, first_non_reserved_inode):
		self.num_blocks               = num_blocks               # total number of blocks (decimal)
		self.num_inodes               = num_inodes               # total number of i-nodes (decimal)
		self.block_size               = block_size               # block size (in bytes, decimal)
		self.inode_size               = inode_size               # i-node size (in bytes, decimal)
		self.num_blocks_per_group     = num_blocks_per_group     # blocks per group (decimal) 
		self.num_inodes_per_group     = num_inodes_per_group     # i-nodes per group (decimal)
		self.first_non_reserved_inode = first_non_reserved_inode # first non-reserved i-node (decimal)

class Group:
	def __init__(self, group_number, num_blocks, num_inodes, num_free_blocks, num_free_inodes, block_num_of_free_block_bitmap, block_num_of_free_inode_bitmap, block_num_of_first_block_of_inode_table):
		self.group_number                            = group_number                            # group number (decimal, starting from zero)
		self.num_blocks                              = num_blocks                              # total number of blocks in this group (decimal)
		self.num_inodes                              = num_inodes                              # total number of i-nodes in this group (decimal)
		self.num_free_blocks                         = num_free_blocks                         # number of free blocks (decimal)
		self.num_free_inodes                         = num_free_inodes                         # number of free i-nodes (decimal)
		self.block_num_of_free_block_bitmap          = block_num_of_free_block_bitmap          # block number of free block bitmap for this group (decimal)
		self.block_num_of_free_inode_bitmap          = block_num_of_free_inode_bitmap          # block number of free i-node bitmap for this group (decimal)
		self.block_num_of_first_block_of_inode_table = block_num_of_first_block_of_inode_table # block number of first block of i-nodes in this group (decimal)

class BFree:
	def __init__(self, free_entry_index):
		self.free_entry_index = free_entry_index # number of the free entry (decimal)

class IFree:
	def __init__(self, free_entry_index):
		self.free_entry_index = free_entry_index # number of the free entry (decimal)

class INode:
	def __init__(self, inode_number, file_type, mode, owner_id, group_id, link_count, creation_timestamp, modification_timestamp, access_timestamp, file_size, num_blocks_of_disk_space, block_numbers):
		self.inode_number             = inode_number                     # inode number (decimal)
		self.file_type                = file_type                        # file type ('f' for file, 'd' for directory, 's' for symbolic link, '?" for anything else)
		self.mode                     = mode                             # mode (low order 12-bits, octal ... suggested format "%o")
		self.owner_id                 = owner_id                         # owner (decimal)
		self.group_id                 = group_id                         # group (decimal)
		self.link_count               = link_count                       # link count (decimal)
		self.creation_timestamp       = creation_timestamp               # time of last I-node change (mm/dd/yy hh:mm:ss, GMT)
		self.modification_timestamp   = modification_timestamp           # modification time (mm/dd/yy hh:mm:ss, GMT)
		self.access_timestamp         = access_timestamp                 # time of last access (mm/dd/yy hh:mm:ss, GMT)
		self.file_size                = file_size                        # file size (decimal)
		self.num_blocks_of_disk_space = num_blocks_of_disk_space         # number of (512 byte) blocks of disk space (decimal) taken up by this file
		self.direct_block_numbers     = block_numbers[:EXT2_NDIR_BLOCKS] # the next fifteen fields are block addresses
		self.indirect_block_numbers   = block_numbers[EXT2_NDIR_BLOCKS:EXT2_N_BLOCKS]


class DirEnt:
	def __init__(self, parent_inode_number, logical_byte_offset, inode_number_of_file, entry_length, name_length, name):
		self.parent_inode_number  = parent_inode_number  # parent inode number (decimal) ... the I-node number of the directory that contains this entry
		self.logical_byte_offset  = logical_byte_offset  # logical byte offset (decimal) of this entry within the directory
		self.inode_number_of_file = inode_number_of_file # inode number of the referenced file (decimal)
		self.entry_length         = entry_length         # entry length (decimal)
		self.name_length          = name_length          # name length (decimal)
		self.name                 = name                 # name (string, surrounded by single-quotes). 

class Indirect:
	def __init__(self, inode_number, level, logical_block_offset, indirect_block_number, block_number):
		self.inode_number          = inode_number          # I-node number of the owning file (decimal)
		self.level                 = level                 # (decimal) level of indirection for the block being scanned 
		self.logical_block_offset  = logical_block_offset  # logical block offset (decimal) represented by the referenced block. 
		self.indirect_block_number = indirect_block_number # block number of the (1, 2, 3) indirect block being scanned (decimal)
		self.block_number          = block_number          # block number of the referenced block (decimal)

class FileSystemImageLoader:
	def __init__(self, filename):
		self.status        = 0
		self.filename      = filename
		self.super_block   = None
		self.group         = None
		self.bfree_list    = []
		self.ifree_list    = []
		self.inode_list    = []
		self.dirent_list   = []
		self.indirect_list = []

	def load(self):
		try:
			with open(self.filename) as csv_file:
				csv_reader = csv.reader(csv_file, delimiter=',')

				for row in csv_reader:
					if   row[0] == 'SUPERBLOCK': self.super_block    = SuperBlock(int(row[1]), int(row[2]), int(row[3]), int(row[4]), int(row[5]), int(row[6]), int(row[7]))
					elif row[0] == 'GROUP':      self.group          = Group(int(row[1]), int(row[2]), int(row[3]), int(row[4]), int(row[5]), int(row[6]), int(row[7]), int(row[8]))
					elif row[0] == 'BFREE':      self.bfree_list    += [BFree(int(row[1]))]
					elif row[0] == 'IFREE':      self.ifree_list    += [IFree(int(row[1]))]
					elif row[0] == 'INODE':      self.inode_list    += [INode(int(row[1]), row[2], row[3], int(row[4]), int(row[5]), int(row[6]), row[7], row[8], row[9], int(row[10]), int(row[11]), list(map(int, row[12:])))]
					elif row[0] == 'DIRENT':     self.dirent_list   += [DirEnt(int(row[1]), int(row[2]), int(row[3]), int(row[4]), int(row[5]), row[6])]
					elif row[0] == 'INDIRECT':   self.indirect_list += [Indirect(int(row[1]), int(row[2]), int(row[3]), int(row[4]), int(row[5]))]

		except IOError:
			self.handle_IOError()
		except IndexError:
			self.handle_IndexError()
		except:
			self.handle_generic_error()

	def handle_IOError(self):
		sys.stderr.write('Error opening the file\n')
		self.status = 1

	def handle_IndexError(self):
		sys.stderr.write('Incorrect file format\n')
		self.status = 1

	def handle_generic_error(self):
		sys.stderr.write('Something went wrong\n')
		self.status = 1

class BlockReference:
	def __init__(self, block_level, block_number, inode_number, block_offset):
		self.block_level  = block_level
		self.block_number = block_number
		self.inode_number = inode_number
		self.block_offset = block_offset

	def get_block_level_string(self):
		level_str = None
		if   (self.block_level == 0): level_str = 'BLOCK'
		elif (self.block_level == 1): level_str = 'INDIRECT BLOCK'
		elif (self.block_level == 2): level_str = 'DOUBLE INDIRECT BLOCK'
		elif (self.block_level == 3): level_str = 'TRIPLE INDIRECT BLOCK'
		return level_str

class FileSystemImageAnalyzer:
	def __init__(self, loader):
		self.status = 0
		self.loader = loader

	def analyze(self):
		self.generate_block_consistency_audits()
		self.generate_inode_allocation_audits()
		self.generate_directory_consistency_audits()

	def generate_block_consistency_audits(self):
		block_num_of_first_non_reserved_block = self.calculate_block_num_of_first_non_reserved_block()
		referenced_blocks = self.check_invalid_reserved_blocks(block_num_of_first_non_reserved_block);
		self.check_unreferenced_allocated_duplicate_blocks(block_num_of_first_non_reserved_block, referenced_blocks);

	def check_invalid_reserved_blocks(self, block_num_of_first_non_reserved_block):
		referenced_blocks = defaultdict(list)

		for inode in self.loader.inode_list:
			for i in range(len(inode.direct_block_numbers)):
				self.check_invalid_reserved_direct_block_number(inode.direct_block_numbers[i], inode.inode_number, i, referenced_blocks, block_num_of_first_non_reserved_block)

			for i in range(len(inode.indirect_block_numbers)):
				self.check_invalid_reserved_indirect_block_number(inode.indirect_block_numbers[i], inode.inode_number, i + 1, referenced_blocks, block_num_of_first_non_reserved_block)
				
		for indirect in self.loader.indirect_list:
			self.check_invalid_reserved_indirect_block_number(indirect.block_number, indirect.inode_number, indirect.level, referenced_blocks, block_num_of_first_non_reserved_block)

		return referenced_blocks

	def check_invalid_reserved_direct_block_number(self, direct_block_number, inode_number, offset, referenced_blocks, block_num_of_first_non_reserved_block):
		if direct_block_number == 0: return

		if self.is_block_number_out_of_bounds(direct_block_number):
			self.print_invalid_direct_block_message(direct_block_number, inode_number, offset)
		elif self.is_block_number_reserved(direct_block_number, block_num_of_first_non_reserved_block):
			self.print_reserved_direct_block_message(direct_block_number, inode_number, offset)
		else:
			referenced_blocks[direct_block_number].append(BlockReference(0, direct_block_number, inode_number, offset))

	def check_invalid_reserved_indirect_block_number(self, indirect_block_number, inode_number, level, referenced_blocks, block_num_of_first_non_reserved_block):
		if indirect_block_number == 0: return

		logical_block_offset = self.calculate_logical_block_offset(level)
		
		if self.is_block_number_out_of_bounds(indirect_block_number):
			self.print_invalid_indirect_block_message(level, indirect_block_number, inode_number, logical_block_offset)
		elif self.is_block_number_reserved(indirect_block_number, block_num_of_first_non_reserved_block):
			self.print_reserved_indirect_block_message(level, indirect_block_number, inode_number, logical_block_offset)
		else:
			referenced_blocks[indirect_block_number].append(BlockReference(level, indirect_block_number, inode_number, logical_block_offset))

	def is_block_number_reserved(self, block_number, block_num_of_first_non_reserved_block):
		return 0 < block_number and block_number < block_num_of_first_non_reserved_block

	def is_block_number_out_of_bounds(self, block_number):
		return block_number < 0 or block_number >= self.loader.super_block.num_blocks

	def check_unreferenced_allocated_duplicate_blocks(self, block_num_of_first_non_reserved_block, referenced_blocks):
		free_block_numbers = self.get_free_block_numbers()

		for block_number in self.get_range_of_unreserved_block_numbers(block_num_of_first_non_reserved_block):
			self.check_unreferenced_block_number(block_number, referenced_blocks, free_block_numbers)
			self.check_allocated_block_number(block_number, referenced_blocks, free_block_numbers)
			self.check_duplicate_block_number(block_number, referenced_blocks)

	def get_range_of_unreserved_block_numbers(self, block_num_of_first_non_reserved_block):
		return range(block_num_of_first_non_reserved_block, self.loader.super_block.num_blocks)

	def check_unreferenced_block_number(self, block_number, referenced_blocks, free_block_numbers):
		if self.is_block_number_unreferenced(block_number, referenced_blocks, free_block_numbers): 
			self.print_unreferenced_block_message(block_number)

	def check_allocated_block_number(self, block_number, referenced_blocks, free_block_numbers):
		if self.is_block_number_allocated(block_number, referenced_blocks, free_block_numbers):
			self.print_allocated_block_on_free_list_message(block_number)

	def check_duplicate_block_number(self, block_number, referenced_blocks):
		block_reference_list = self.get_duplicate_block_references(block_number, referenced_blocks)
		if block_reference_list:
			for block_reference in block_reference_list:
				self.print_duplicate_reference_message(block_reference)

	def is_block_number_unreferenced(self, block_number, referenced_blocks, free_block_numbers):
		return block_number not in referenced_blocks and block_number not in free_block_numbers

	def is_block_number_allocated(self, block_number, referenced_blocks, free_block_numbers):
		return block_number in referenced_blocks and block_number in free_block_numbers

	def get_duplicate_block_references(self, block_number, referenced_blocks):
		if block_number not in referenced_blocks: return []
		block_reference_list = referenced_blocks[block_number]
		if len(block_reference_list) == 1: return []
		return block_reference_list

	def generate_inode_allocation_audits(self):
		free_inode_numbers = self.get_free_inode_numbers()
		allocated_inode_numbers = self.get_allocated_inode_numbers()

		self.check_allocated_inode(allocated_inode_numbers, free_inode_numbers)
		self.check_unallocated_inode(allocated_inode_numbers, free_inode_numbers)
		

	def check_allocated_inode(self, allocated_inode_numbers, free_inode_numbers):
		for inode_number in allocated_inode_numbers:
			if inode_number in free_inode_numbers: self.print_allocated_inode_is_free_message(inode_number)

	def check_unallocated_inode(self, allocated_inode_numbers, free_inode_numbers):
		for inode_number in self.get_range_of_unreserved_inode_numbers():
			if inode_number not in allocated_inode_numbers and inode_number not in free_inode_numbers: self.print_unallocated_inode_is_not_free_message(inode_number)

	def generate_directory_consistency_audits(self):
		link_count_dict, parent_dict = self.check_directory_entries()
		self.check_inode_link_count(link_count_dict)
		self.check_special_directory_entries(parent_dict)		

	def check_directory_entries(self):
		allocated_inode_numbers = self.get_allocated_inode_numbers()
		link_count_dict = defaultdict(int)
		parent_dict = {}
		
		for dirent in self.loader.dirent_list: 
			if self.is_inode_number_out_of_bounds(dirent.inode_number_of_file):
				self.print_invalid_directory_entry_message(dirent.parent_inode_number, dirent.name, dirent.inode_number_of_file)
			elif dirent.inode_number_of_file not in allocated_inode_numbers:
				self.print_unallocated_directory_entry_message(dirent.parent_inode_number, dirent.name, dirent.inode_number_of_file)
			else:
				link_count_dict[dirent.inode_number_of_file] += 1
				if dirent.inode_number_of_file not in parent_dict: parent_dict[dirent.inode_number_of_file] = dirent.parent_inode_number

		return link_count_dict, parent_dict

	def is_inode_number_out_of_bounds(self, inode_number):
		return inode_number < 1 or inode_number > self.loader.super_block.num_inodes

	def check_inode_link_count(self, link_count_dict):
		for inode in self.loader.inode_list:
			if inode.link_count != link_count_dict[inode.inode_number]:
				self.print_inode_link_count_inconsistency_message(inode.inode_number, link_count_dict[inode.inode_number], inode.link_count)

	def check_special_directory_entries(self, parent_dict):
		for dirent in self.loader.dirent_list:
			if dirent.name == "'.'" and dirent.parent_inode_number != dirent.inode_number_of_file:
				self.print_special_directory_inode_number_inconsistency_message(dirent.parent_inode_number, dirent.name, dirent.inode_number_of_file, dirent.parent_inode_number)
			elif dirent.name == "'..'" and parent_dict[dirent.parent_inode_number] != dirent.inode_number_of_file:
				self.print_special_directory_inode_number_inconsistency_message(dirent.parent_inode_number, dirent.name, dirent.inode_number_of_file, parent_dict[dirent.parent_inode_number])

	def print_invalid_direct_block_message(self, direct_block_number, inode_number, block_offset):
		print('INVALID BLOCK {} IN INODE {} AT OFFSET {}'.format(direct_block_number, inode_number, block_offset))
		self.status = 2

	def print_reserved_direct_block_message(self, direct_block_number, inode_number, block_offset):
		print('RESERVED BLOCK {} IN INODE {} AT OFFSET {}'.format(direct_block_number, inode_number, block_offset))
		self.status = 2

	def print_invalid_indirect_block_message(self, level, indirect_block_number, inode_number, block_offset):
		print('INVALID {} BLOCK {} IN INODE {} AT OFFSET {}'.format(self.generate_level_string(level), indirect_block_number, inode_number, block_offset))
		self.status = 2

	def print_reserved_indirect_block_message(self, level, indirect_block_number, inode_number, block_offset):
		print('RESERVED {} BLOCK {} IN INODE {} AT OFFSET {}'.format(self.generate_level_string(level), indirect_block_number, inode_number, block_offset))
		self.status = 2

	def print_unreferenced_block_message(self, block_number):
		print('UNREFERENCED BLOCK {}'.format(block_number))
		self.status = 2

	def print_allocated_block_on_free_list_message(self, block_number):
		print('ALLOCATED BLOCK {} ON FREELIST'.format(block_number))
		self.status = 2

	def print_duplicate_reference_message(self, block_reference):
		print('DUPLICATE {} {} IN INODE {} AT OFFSET {}'.format(block_reference.get_block_level_string(), block_reference.block_number, block_reference.inode_number, block_reference.block_offset))
		self.status = 2

	def print_allocated_inode_is_free_message(self, inode_number):
		print('ALLOCATED INODE {} ON FREELIST'.format(inode_number))
		self.status = 2

	def print_unallocated_inode_is_not_free_message(self, inode_number):
		print('UNALLOCATED INODE {} NOT ON FREELIST'.format(inode_number))
		self.status = 2

	def print_invalid_directory_entry_message(self, parent_inode_number, directory_entry_name, inode_number_of_file):
		print('DIRECTORY INODE {} NAME {} INVALID INODE {}'.format(parent_inode_number, directory_entry_name, inode_number_of_file))
		self.status = 2

	def print_unallocated_directory_entry_message(self, parent_inode_number, directory_entry_name, inode_number_of_file):
		print('DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}'.format(parent_inode_number, directory_entry_name, inode_number_of_file))
		self.status = 2

	def print_inode_link_count_inconsistency_message(self, inode_number, actual_link_count, expected_link_count):
		print('INODE {} HAS {} LINKS BUT LINKCOUNT IS {}'.format(inode_number, actual_link_count, expected_link_count))
		self.status = 2

	def print_special_directory_inode_number_inconsistency_message(self, parent_inode_number, entry_name, inode_number_of_file, expected_inode_number):
		print('DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}'.format(parent_inode_number, entry_name, inode_number_of_file, expected_inode_number))
		self.status = 2

	def get_range_of_unreserved_inode_numbers(self):
		return range(self.loader.super_block.first_non_reserved_inode, self.loader.super_block.num_inodes + 1)

	def generate_level_string(self, level):
		level_str = None
		if   (level == 1): level_str = 'INDIRECT'
		elif (level == 2): level_str = 'DOUBLE INDIRECT'
		elif (level == 3): level_str = 'TRIPLE INDIRECT'
		return level_str

	def calculate_logical_block_offset(self, level):
		logical_block_offset = None
		if   (level == 1): logical_block_offset = EXT2_NDIR_BLOCKS
		elif (level == 2): logical_block_offset = EXT2_NDIR_BLOCKS + NUM_BLOCK_NUMBER_IN_A_BLOCK
		elif (level == 3): logical_block_offset = EXT2_NDIR_BLOCKS + (1 + NUM_BLOCK_NUMBER_IN_A_BLOCK) * NUM_BLOCK_NUMBER_IN_A_BLOCK
		return logical_block_offset

	def calculate_block_num_of_first_non_reserved_block(self):
		return self.loader.group.block_num_of_first_block_of_inode_table + self.loader.super_block.inode_size * self.loader.group.num_inodes / self.loader.super_block.block_size

	def get_free_block_numbers(self):
		return list(map(lambda bfree: bfree.free_entry_index, self.loader.bfree_list))

	def get_free_inode_numbers(self):
		return list(map(lambda ifree: ifree.free_entry_index, self.loader.ifree_list))

	def get_allocated_inode_numbers(self):
		return list(map(lambda inode: inode.inode_number, self.loader.inode_list))


def get_usage():
	return "\
Usage: ./lab3b [FILE SYSTEM SUMMARY CSV]\n\
analyze a file system summary and report on all discovered inconsistencies.\n";

def handle_unrecognized_arguments():
	sys.stderr.write('Unrecognized arguments detected\n')
	sys.stderr.write(get_usage())
	sys.exit(1)

def main():
	if len(sys.argv) != 2: handle_unrecognized_arguments()

	loader = FileSystemImageLoader(sys.argv[1])
	loader.load()

	if (loader.status == 1): sys.exit(1);

	analyzer = FileSystemImageAnalyzer(loader)
	analyzer.analyze()

	sys.exit(analyzer.status)

if __name__ == '__main__':
	main()






