#include "simplefs-ops.h"

// Array for storing opened files
extern struct filehandle_t file_handle_array[MAX_OPEN_FILES];

// Create file with name `filename` from disk
int simplefs_create(char *filename) {
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
  int inodenum;

  // Iterate thru all inodes
  for (inodenum = 0; inodenum < NUM_INODES; inodenum++) {
    // Read the inode
    simplefs_readInode(inodenum, inode);

    // If already free, ignore it
    if (inode->status == INODE_FREE)
      continue;

    // If not free then check if the name matches
    if (!strcmp(inode->name, filename)) {
      free(inode);
      return 1;
    }
  }

  // Allocate inode if it is feasible
  inodenum = simplefs_allocInode();
  if (inodenum == -1) {
    free(inode); // Free malloced data
    return -1;
  }

  // Setup the inode
  inode->status = INODE_IN_USE;
  inode->file_size = 0;
  for (int i = 0; i < MAX_FILE_SIZE; i++)
    inode->direct_blocks[i] = -1;
  strcpy(inode->name, filename);

  // Write the inode
  simplefs_writeInode(inodenum, inode);

  free(inode); // Free malloced data
  return inodenum;
}

// delete file with name `filename` from disk
void simplefs_delete(char *filename) {
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
  int inodenum;

  // Iterate thru all inodes
  for (inodenum = 0; inodenum < NUM_INODES; inodenum++) {
    // Read the inode
    simplefs_readInode(inodenum, inode);

    // If already free, ignore it
    if (inode->status == INODE_FREE)
      continue;

    // If not free then check if the name matches
    if (!strcmp(inode->name, filename)) {
      break;
    }
  }

  // If match not found, do nothing
  if (inodenum == NUM_BLOCKS) {
    free(inode); // Free malloced data
    return;
  }

  // If match found then free data blocks and inode itself
  for (int j = 0; j < MAX_FILE_SIZE; j++) {
    if (inode->direct_blocks[j] == -1)
      continue;
    simplefs_freeDataBlock(inode->direct_blocks[j]);
  }
  simplefs_freeInode(inodenum);

  free(inode); // Free malloced data
  return;
}

// open file with name `filename`
int simplefs_open(char *filename) {
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
  int inodenum;

  // Iterate thru all inodes
  for (inodenum = 0; inodenum < NUM_INODES; inodenum++) {
    // Read the inode
    simplefs_readInode(inodenum, inode);

    // If already free, ignore it
    if (inode->status == INODE_FREE)
      continue;

    // If not free then check if the name matches
    if (!strcmp(inode->name, filename)) {
      break;
    }
  }
  free(inode); // Free malloced data

  // If match not found, do nothing
  if (inodenum == NUM_BLOCKS)
    return -1;

  // Check free file handle and assign it
  int file_handle;
  for (file_handle = 0; file_handle < MAX_OPEN_FILES; file_handle++) {
    if (file_handle_array[file_handle].inode_number != -1)
      continue;
    file_handle_array[file_handle].inode_number = inodenum;
    file_handle_array[file_handle].offset = 0;
    break;
  }

  // No file handle is free, do nothing
  if (file_handle == MAX_OPEN_FILES)
    return -1;

  // If file handle assigned then return index
  return file_handle;
}

// close file pointed by `file_handle`
void simplefs_close(int file_handle) {
  // Check if the file handle is feasible
  if (file_handle >= MAX_OPEN_FILES)
    return;

  // Reset the file handle
  file_handle_array[file_handle].inode_number = -1;
  file_handle_array[file_handle].offset = 0;
  return;
}

// read `nbytes` of data into `buf` from file pointed by `file_handle`
// starting at current offset
int simplefs_read(int file_handle, char *buf, int nbytes) {
  // If nbytes isn't positive, it is invalid
  if (nbytes <= 0)
    return -1;

  // Get the offset and the inode number
  int offset = file_handle_array[file_handle].offset;
  int inodenum = file_handle_array[file_handle].inode_number;
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));

  // Read the inode
  simplefs_readInode(inodenum, inode);

  // If read crosses boundary, do nothing
  if (inode->file_size < offset + nbytes) {
    free(inode); // Free malloced data
    return -1;
  }

  char tempBlockBuf[BLOCKSIZE];
  int tempset = 0;
  // Iterate over all possible data blocks
  for (int i = 0; i < NUM_DATA_BLOCKS; i++) {
    // If found an allocated block, break loop
    if (inode->direct_blocks[i] == -1)
      break;

    // If end is before the block, break loop
    if (offset + nbytes < i * BLOCKSIZE)
      break;

    // If we don't need this block then skip
    if (offset + tempset > (i + 1) * BLOCKSIZE)
      continue;

    // Set ls as left side diff and rs as right side diff
    int ls = offset - (i * BLOCKSIZE);
    int rs = (i + 1) * BLOCKSIZE - (offset + nbytes);
    if (ls < 0)
      ls = 0;
    if (rs < 0)
      rs = 0;

    // Read the data block
    simplefs_readDataBlock(inode->direct_blocks[i], tempBlockBuf);

    // Copy the required portion of the data block
    memcpy(buf + tempset, tempBlockBuf + ls, BLOCKSIZE - ls - rs);

    // Update the tempset
    tempset += BLOCKSIZE - ls - rs;
  }

  free(inode); // Free malloced data
  return 0;
}

// write `nbytes` of data from `buf` to file pointed by `file_handle`
// starting at current offset
int simplefs_write(int file_handle, char *buf, int nbytes) {
  // If nbytes isn't positive, it is invalid
  if (nbytes <= 0)
    return -1;

  // Get the offset and the inode number
  int offset = file_handle_array[file_handle].offset;
  int inodenum = file_handle_array[file_handle].inode_number;
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));

  // Read the inode
  simplefs_readInode(inodenum, inode);

  // Compute the required blocks
  int req_blocks = (offset + nbytes - 1) / BLOCKSIZE + 1;

  // If read crosses boundary, do nothing
  if (req_blocks > MAX_FILE_SIZE) {
    free(inode); // Free malloced data
    return -1;
  }

  int first_new = MAX_FILE_SIZE;
  // Start allocating new blocks
  for (int i = 0; i < req_blocks; i++) {
    // Ignore blocks which are already allocated
    if (inode->direct_blocks[i] != -1)
      continue;

    // Save the index of first block which needs to be allocated
    if (first_new == MAX_FILE_SIZE)
      first_new = i;

    // Allocate the block if it is feasible
    inode->direct_blocks[i] = simplefs_allocDataBlock();

    // Continue if the allocation succeeds
    if (inode->direct_blocks[i] != -1)
      continue;

    // If the allocation fails then revert back thing
    for (i--; i >= first_new; i--) {
      simplefs_freeDataBlock(inode->direct_blocks[i]);
      inode->direct_blocks[i] = -1;
    }
    free(inode); // Free malloced data
    return -1;
  }

  // Update the file size
  if (inode->file_size < offset + nbytes)
    inode->file_size = offset + nbytes;

  // Write the inode
  simplefs_writeInode(inodenum, inode);

  char tempBlockBuf[BLOCKSIZE];
  int tempset = 0;
  // Iterate over all possible data blocks
  for (int i = 0; i < MAX_FILE_SIZE; i++) {
    // If found an allocated block, break loop
    if (inode->direct_blocks[i] == -1)
      break;

    // If end is before the block, break loop
    if (offset + nbytes <= i * BLOCKSIZE)
      break;

    // Set is_new
    int is_new = 0;
    if (first_new <= i)
      is_new = 1;

    // Case 1
    if (offset + tempset == i * BLOCKSIZE &&
        offset + nbytes >= (i + 1) * BLOCKSIZE) {
      if (!is_new) {
        simplefs_readDataBlock(inode->direct_blocks[i], tempBlockBuf);
      }
      memcpy(tempBlockBuf, buf + tempset, BLOCKSIZE);
      simplefs_writeDataBlock(inode->direct_blocks[i], tempBlockBuf);
      tempset += BLOCKSIZE;
      continue;
    }

    // Case 2
    if (offset + tempset > i * BLOCKSIZE &&
        offset + tempset <= (i + 1) * BLOCKSIZE &&
        offset + nbytes >= (i + 1) * BLOCKSIZE) {
      int ls = offset - (i * BLOCKSIZE);
      if (!is_new) {
        simplefs_readDataBlock(inode->direct_blocks[i], tempBlockBuf);
      }
      memcpy(tempBlockBuf + ls, buf + tempset, BLOCKSIZE - ls);
      simplefs_writeDataBlock(inode->direct_blocks[i], tempBlockBuf);
      tempset += BLOCKSIZE - ls;
      continue;
    }

    // Case 3
    if (offset + tempset == i * BLOCKSIZE &&
        offset + nbytes < (i + 1) * BLOCKSIZE) {
      int rs = (i + 1) * BLOCKSIZE - (offset + nbytes);
      if (!is_new) {
        simplefs_readDataBlock(inode->direct_blocks[i], tempBlockBuf);
      }
      memcpy(tempBlockBuf, buf + tempset, BLOCKSIZE - rs);
      simplefs_writeDataBlock(inode->direct_blocks[i], tempBlockBuf);
      tempset += BLOCKSIZE - rs;
      continue;
    }

    // Case 4
    if (offset + tempset > i * BLOCKSIZE &&
        offset + nbytes < (i + 1) * BLOCKSIZE) {
      int ls = offset - (i * BLOCKSIZE);
      int rs = (i + 1) * BLOCKSIZE - (offset + nbytes);
      if (!is_new) {
        simplefs_readDataBlock(inode->direct_blocks[i], tempBlockBuf);
      }
      memcpy(tempBlockBuf + ls, buf + tempset, BLOCKSIZE - ls - rs);
      simplefs_writeDataBlock(inode->direct_blocks[i], tempBlockBuf);
      tempset += BLOCKSIZE - ls - rs;
      continue;
    }
  }

  free(inode); // Free malloced data
  return 0;
}

// increase `file_handle` offset by `nseek`
int simplefs_seek(int file_handle, int nseek) {
  // Check if the file handle is feasible
  if (file_handle >= MAX_OPEN_FILES)
    return -1;

  // Find the new offset
  int new_offset = file_handle_array[file_handle].offset + nseek;

  // If new offset is negative, do nothing
  if (new_offset < 0)
    return -1;

  // Get the inode number
  int inodenum = file_handle_array[file_handle].inode_number;
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));

  // Read the inode
  simplefs_readInode(inodenum, inode);

  // If new offset crosses file size, do nothing
  if (new_offset > inode->file_size) {
    free(inode); // Free malloced data
    return -1;
  }

  // Update the offset in file handle
  file_handle_array[file_handle].offset = new_offset;

  free(inode); // Free malloced data
  return 0;
}
