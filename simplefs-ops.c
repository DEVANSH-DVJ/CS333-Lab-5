#include "simplefs-ops.h"

// Array for storing opened files
extern struct filehandle_t file_handle_array[MAX_OPEN_FILES];

//  Create file with name `filename` from disk
int simplefs_create(char *filename) {
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
  int inodenum;

  for (inodenum = 0; inodenum < NUM_INODES; inodenum++) {
    simplefs_readInode(inodenum, inode);
    if (inode->status == INODE_FREE)
      continue;
    if (!strcmp(inode->name, filename)) {
      free(inode);
      return 1;
    }
  }

  inodenum = simplefs_allocInode();
  if (inodenum == -1) {
    free(inode);
    return -1;
  }

  inode->status = INODE_IN_USE;
  inode->file_size = 0;
  for (int i = 0; i < MAX_FILE_SIZE; i++)
    inode->direct_blocks[i] = -1;
  strcpy(inode->name, filename);
  simplefs_writeInode(inodenum, inode);
  free(inode);

  return inodenum;
}

//  delete file with name `filename` from disk
void simplefs_delete(char *filename) {
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
  int inodenum;

  for (inodenum = 0; inodenum < NUM_INODES; inodenum++) {
    simplefs_readInode(inodenum, inode);
    if (inode->status == INODE_FREE)
      continue;
    if (!strcmp(inode->name, filename)) {
      break;
    }
  }

  if (inodenum == NUM_BLOCKS) {
    free(inode);
    return;
  }

  for (int j = 0; j < MAX_FILE_SIZE; j++) {
    if (inode->direct_blocks[j] == -1)
      continue;
    simplefs_freeDataBlock(inode->direct_blocks[j]);
  }
  simplefs_freeInode(inodenum);
  free(inode);

  return;
}

//  open file with name `filename`
int simplefs_open(char *filename) {
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
  int inodenum;

  for (inodenum = 0; inodenum < NUM_INODES; inodenum++) {
    simplefs_readInode(inodenum, inode);
    if (inode->status == INODE_FREE)
      continue;
    if (!strcmp(inode->name, filename)) {
      break;
    }
  }
  free(inode);

  if (inodenum == NUM_BLOCKS)
    return -1;

  int file_handle;
  for (file_handle = 0; file_handle < MAX_OPEN_FILES; file_handle++) {
    if (file_handle_array[file_handle].inode_number != -1)
      continue;
    file_handle_array[file_handle].inode_number = inodenum;
    file_handle_array[file_handle].offset = 0;
    break;
  }

  if (file_handle == MAX_OPEN_FILES)
    return -1;
  return file_handle;
}

//  close file pointed by `file_handle`
void simplefs_close(int file_handle) {
  if (file_handle >= MAX_OPEN_FILES)
    return;

  file_handle_array[file_handle].inode_number = -1;
  file_handle_array[file_handle].offset = 0;
  return;
}

//  read `nbytes` of data into `buf` from file pointed by `file_handle`
//  starting at current offset
int simplefs_read(int file_handle, char *buf, int nbytes) {
  if (nbytes <= 0)
    return -1;

  int offset = file_handle_array[file_handle].offset;
  int inodenum = file_handle_array[file_handle].inode_number;
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
  simplefs_readInode(inodenum, inode);

  if (inode->file_size < offset + nbytes) {
    free(inode);
    return -1;
  }

  char tempBuf[nbytes];
  char tempBlockBuf[BLOCKSIZE];
  int tempset = 0;
  for (int i = 0; i < NUM_DATA_BLOCKS; i++) {
    if (inode->direct_blocks[i] == -1)
      break;

    if (offset + nbytes < i * BLOCKSIZE)
      break;

    if (offset + tempset == i * BLOCKSIZE &&
        offset + nbytes >= (i + 1) * BLOCKSIZE) {
      simplefs_readDataBlock(inode->direct_blocks[i], tempBlockBuf);
      memcpy(tempBuf + tempset, tempBlockBuf, BLOCKSIZE);
      tempset += BLOCKSIZE;
      continue;
    }

    if (offset + tempset > i * BLOCKSIZE &&
        offset + tempset <= (i + 1) * BLOCKSIZE &&
        offset + nbytes >= (i + 1) * BLOCKSIZE) {
      int ls = offset - (i * BLOCKSIZE);
      simplefs_readDataBlock(inode->direct_blocks[i], tempBlockBuf);
      memcpy(tempBuf + tempset, tempBlockBuf + ls, BLOCKSIZE - ls);
      tempset += BLOCKSIZE - ls;
      continue;
    }

    if (offset + tempset == i * BLOCKSIZE &&
        offset + nbytes < (i + 1) * BLOCKSIZE) {
      int rs = (i + 1) * BLOCKSIZE - (offset + nbytes);
      simplefs_readDataBlock(inode->direct_blocks[i], tempBlockBuf);
      memcpy(tempBuf + tempset, tempBlockBuf, BLOCKSIZE - rs);
      tempset += BLOCKSIZE - rs;
      continue;
    }

    if (offset + tempset > i * BLOCKSIZE &&
        offset + nbytes < (i + 1) * BLOCKSIZE) {
      int ls = offset - (i * BLOCKSIZE);
      int rs = (i + 1) * BLOCKSIZE - (offset + nbytes);
      simplefs_readDataBlock(inode->direct_blocks[i], tempBlockBuf);
      memcpy(tempBuf + tempset, tempBlockBuf + ls, BLOCKSIZE - ls - rs);
      tempset += BLOCKSIZE - ls - rs;
      continue;
    }
  }

  free(inode);
  memcpy(buf, tempBuf, nbytes);
  return 0;
}

//  write `nbytes` of data from `buf` to file pointed by `file_handle`
//  starting at current offset
int simplefs_write(int file_handle, char *buf, int nbytes) {
  if (nbytes <= 0)
    return -1;

  int offset = file_handle_array[file_handle].offset;
  int inodenum = file_handle_array[file_handle].inode_number;
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
  simplefs_readInode(inodenum, inode);

  int req_blocks = (offset + nbytes) / BLOCKSIZE;
  if (req_blocks > MAX_FILE_SIZE) {
    free(inode);
    return -1;
  }

  int first_new = -1;
  for (int i = 0; i < req_blocks; i++) {
    if (inode->direct_blocks[i] != -1)
      continue;

    if (first_new == -1)
      first_new = i;

    inode->direct_blocks[i] = simplefs_allocDataBlock();
    if (inode->direct_blocks[i] != -1)
      continue;

    for (; i >= first_new; i--) {
      simplefs_freeDataBlock(inode->direct_blocks[i]);
      inode->direct_blocks[i] = -1;
    }
    free(inode);
    return -1;
  }
  if (inode->file_size < offset + nbytes)
    inode->file_size = offset + nbytes;
  simplefs_writeInode(inodenum, inode);

  char tempBlockBuf[BLOCKSIZE];
  int tempset = 0;
  for (int i = 0; i < NUM_DATA_BLOCKS; i++) {
    if (inode->direct_blocks[i] == -1)
      break;

    if (offset + nbytes < i * BLOCKSIZE)
      break;

    int is_new = 0;
    if (first_new <= i)
      is_new = 1;

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

  free(inode);
  return 0;
}

// increase `file_handle` offset by `nseek`
int simplefs_seek(int file_handle, int nseek) {
  int new_offset = file_handle_array[file_handle].offset + nseek;

  if (new_offset < 0)
    return -1;

  int inodenum = file_handle_array[file_handle].inode_number;
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
  simplefs_readInode(inodenum, inode);
  if (new_offset > inode->file_size) {
    free(inode);
    return -1;
  }

  file_handle_array[file_handle].offset = new_offset;
  free(inode);
  return 0;
}
