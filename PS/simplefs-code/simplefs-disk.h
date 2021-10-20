/*
	DISK EMULATION
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#define BLOCKSIZE 64
#define NUM_BLOCKS 35
#define NUM_DATA_BLOCKS 30
#define NUM_INODE_BLOCKS 4
#define NUM_INODES 8
#define NUM_INODES_PER_BLOCK 2
#define MAX_FILE_SIZE 4 // In Blocks
#define MAX_FILES 8
#define MAX_OPEN_FILES 20
#define MAX_NAME_STRLEN 8
#define INODE_FREE 'x'
#define INODE_IN_USE '1'
#define DATA_BLOCK_FREE 'x'
#define DATA_BLOCK_USED '1'

struct superblock_t
{
	char name[MAX_NAME_STRLEN]; 				// "simplefs" after formatting
	char inode_freelist[NUM_INODES];			// INODE_FREE if free, INODE_IN_USE if used
	char datablock_freelist[NUM_DATA_BLOCKS];   // DATA_BLOCK_FREE if free, DATA_BLOCK_USED if used
};

struct inode_t
{
	int status;								// INODE_FREE if free, INODE_IN_USE if used
	char name[MAX_NAME_STRLEN];					// name of the file
	int file_size;								// size of the file in bytes
	int direct_blocks[MAX_FILE_SIZE];			// -1 if free, block number if used
};

struct filehandle_t
{
	int offset;		  // current offset in opened file
	int inode_number; // Inode number for the file
};

void simplefs_formatDisk();
int simplefs_allocInode();
void simplefs_freeInode(int inodenum);
void simplefs_readInode(int inodenum, struct inode_t *inodeptr);
void simplefs_writeInode(int inodenum, struct inode_t *inodeptr); 
int simplefs_allocDataBlock();
void simplefs_freeDataBlock(int blocknum);
void simplefs_readDataBlock(int blocknum, char *buf);
void simplefs_writeDataBlock(int blocknum, char *buf);
void simplefs_dump();
