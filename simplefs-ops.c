#include "simplefs-ops.h"

// Array for storing opened files
extern struct filehandle_t file_handle_array[MAX_OPEN_FILES];

//  Create file with name `filename` from disk
int simplefs_create(char *filename) {
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
  int inode_no;

  for (inode_no = 0; inode_no < NUM_INODES; inode_no++) {
    simplefs_readInode(inode_no, inode);
    if (inode->status == INODE_FREE)
      continue;
    if (!strcmp(inode->name, filename)) {
      free(inode);
      return 1;
    }
  }

  inode_no = simplefs_allocInode();
  if (inode_no == -1) {
    free(inode);
    return -1;
  }

  inode->status = INODE_IN_USE;
  inode->file_size = 0;
  for (int i = 0; i < MAX_FILE_SIZE; i++)
    inode->direct_blocks[i] = -1;
  strcpy(inode->name, filename);
  simplefs_writeInode(inode_no, inode);
  free(inode);

  return inode_no;
}

//  delete file with name `filename` from disk
void simplefs_delete(char *filename) {
  struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
  int inode_no;

  for (inode_no = 0; inode_no < NUM_INODES; inode_no++) {
    simplefs_readInode(inode_no, inode);
    if (inode->status == INODE_FREE)
      continue;
    if (!strcmp(inode->name, filename)) {
      break;
    }
  }

  if (inode_no == NUM_BLOCKS)
    return;

  for (int j = 0; j < MAX_FILE_SIZE; j++) {
    if (inode->direct_blocks[j] == -1)
      continue;
    simplefs_freeDataBlock(inode->direct_blocks[j]);
  }
  simplefs_freeInode(inode_no);
  free(inode);

  return;
}

int simplefs_open(char *filename) {
  //  open file with name `filename`
  return -1;
}

void simplefs_close(int file_handle) {
  //  close file pointed by `file_handle`
}

int simplefs_read(int file_handle, char *buf, int nbytes) {
  //  read `nbytes` of data into `buf` from file pointed by `file_handle`
  //  starting at current offset
  return -1;
}

int simplefs_write(int file_handle, char *buf, int nbytes) {
  //  write `nbytes` of data from `buf` to file pointed by `file_handle`
  //  starting at current offset
  return -1;
}

int simplefs_seek(int file_handle, int nseek) {
  // increase `file_handle` offset by `nseek`
  return -1;
}
