#include "simplefs-disk.h"

int DISK_FD;   // pointer to simplefs.txt
struct filehandle_t file_handle_array[MAX_OPEN_FILES]; // Array for storing opened files

void simplefs_readSuperBlock(struct superblock_t *superblock){
    /*
	    Helper function to read superblock from disk into superblock_t structure
	*/
    char tempBuf[BLOCKSIZE];
    lseek(DISK_FD, 0, SEEK_SET);
    int ret = read(DISK_FD ,tempBuf, BLOCKSIZE);
    assert(ret == BLOCKSIZE);
    memcpy(superblock, tempBuf, sizeof(struct superblock_t));
}

void simplefs_writeSuperBlock(struct superblock_t *superblock){
    /*
	    Helper function to write superblock from superblock_t structure to disk
	*/
    char tempBuf[BLOCKSIZE];
    memcpy(tempBuf, superblock, sizeof(struct superblock_t));
    lseek(DISK_FD, 0, SEEK_SET);
    int ret = write(DISK_FD ,tempBuf, BLOCKSIZE);
    assert(ret == BLOCKSIZE);
}

void simplefs_formatDisk(){
    /*
	    Format filesystem and initialise superblock and inodes with default values
	*/

    FILE* fp;
    fp = fopen("simplefs", "w+");
    DISK_FD = fileno(fp);

    // Setting up superblock
    struct superblock_t *superblock = (struct superblock_t *)malloc(sizeof(struct superblock_t));
    memcpy(superblock->name, "simplefs", 8);
    for(int i=0; i<NUM_INODES; i++)
        superblock->inode_freelist[i] = INODE_FREE;
    for(int i=0; i<NUM_DATA_BLOCKS; i++){
        superblock->datablock_freelist[i] = DATA_BLOCK_FREE;
    }
    simplefs_writeSuperBlock(superblock);
    free(superblock);
    
    // Setting up inode structure
    struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
    memcpy(inode->name, "", 1);
    inode->status = INODE_FREE;
    inode->file_size = 0;
    for(int i=0; i<MAX_FILE_SIZE; i++)
        inode->direct_blocks[i] = -1;
    for(int i=0; i<NUM_INODES; i++)
        simplefs_writeInode(i, inode);
    free(inode);

    // Formatting file handler array
    for(int i=0; i<MAX_OPEN_FILES; i++){
        file_handle_array[i].inode_number = -1;
        file_handle_array[i].offset = 0;
    }
}

int simplefs_allocInode(){
    /*
	    Iterate over `inode_freelist` and return index of first empty inode
	*/
    struct superblock_t *superblock = (struct superblock_t *)malloc(sizeof(struct superblock_t));
    simplefs_readSuperBlock(superblock);
    for(int i=0; i<NUM_INODES; i++){
        if(superblock->inode_freelist[i] == INODE_FREE){
            superblock->inode_freelist[i] = INODE_IN_USE;
            simplefs_writeSuperBlock(superblock);
            free(superblock);
            return i;
        }
    }
    free(superblock);
    return -1;
}

void simplefs_freeInode(int inodenum){
    /*
	    free inode with index `inodenum`     
	*/
    assert(inodenum < NUM_INODES);
    struct superblock_t *superblock = (struct superblock_t *)malloc(sizeof(struct superblock_t));
    struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
    simplefs_readSuperBlock(superblock);
    simplefs_readInode(inodenum, inode);
    assert(superblock->inode_freelist[inodenum] == INODE_IN_USE);
    superblock->inode_freelist[inodenum] = INODE_FREE;
    inode->status = INODE_FREE;
    inode->file_size = 0;
    for (int i = 0; i < MAX_FILE_SIZE; i++)
        inode->direct_blocks[i] = -1;
    simplefs_writeSuperBlock(superblock);
    simplefs_writeInode(inodenum, inode);
    free(inode);
    free(superblock);
}

void simplefs_readInode(int inodenum, struct inode_t *inodeptr){
    /*
	    read inode with index `inodenum` from disk into `inodeptr`     
	*/
    assert(inodenum < NUM_INODES);
    char tempBuf[BLOCKSIZE / NUM_INODES_PER_BLOCK];
    lseek(DISK_FD, BLOCKSIZE + inodenum * sizeof(struct inode_t), SEEK_SET);
    int ret = read(DISK_FD, tempBuf, sizeof(struct inode_t));
    assert(ret == sizeof(struct inode_t));
    memcpy(inodeptr, tempBuf, sizeof(struct inode_t));
}

void simplefs_writeInode(int inodenum, struct inode_t *inodeptr){
    /*
	    write `inodeptr` to inode with index `inodenum` on disk    
	*/
    assert(inodenum < NUM_INODES);
    char tempBuf[BLOCKSIZE / NUM_INODES_PER_BLOCK];
    memcpy(tempBuf, inodeptr, sizeof(struct inode_t));
    lseek(DISK_FD, BLOCKSIZE + inodenum * sizeof(struct inode_t), SEEK_SET);
    int ret = write(DISK_FD, tempBuf, sizeof(struct inode_t));
    assert(ret == sizeof(struct inode_t));
}

int simplefs_allocDataBlock(){
    /*
	    Iterate over `datablock_freelist` and return index of first empty inode
	*/
    struct superblock_t *superblock = (struct superblock_t *)malloc(sizeof(struct superblock_t));
    simplefs_readSuperBlock(superblock);
    for (int i = 0; i < NUM_DATA_BLOCKS; i++){
        if (superblock->datablock_freelist[i] == DATA_BLOCK_FREE){
            superblock->datablock_freelist[i] = DATA_BLOCK_USED;
            simplefs_writeSuperBlock(superblock);
            free(superblock);
            return i;
        }
    }
    free(superblock);   
    return -1;
}

void simplefs_freeDataBlock(int blocknum){
    /*
	    free data block with index `blocknum`     
	*/
    struct superblock_t *superblock = (struct superblock_t *)malloc(sizeof(struct superblock_t));
    simplefs_readSuperBlock(superblock);
    assert(superblock->datablock_freelist[blocknum] == DATA_BLOCK_USED);
    superblock->datablock_freelist[blocknum] = DATA_BLOCK_FREE;
    simplefs_writeSuperBlock(superblock);
    free(superblock);
}

void simplefs_readDataBlock(int blocknum, char *buf){
    /*
	    read data block with index `blocknum` from disk into `buf`     
	*/
    assert(blocknum < NUM_DATA_BLOCKS);
    char tempBuf[BLOCKSIZE];
    lseek(DISK_FD, BLOCKSIZE*(5 + blocknum), SEEK_SET);
    int ret = read(DISK_FD, tempBuf, BLOCKSIZE);
    assert(ret == BLOCKSIZE);
    memcpy(buf, tempBuf, BLOCKSIZE);
}

void simplefs_writeDataBlock(int blocknum, char *buf){
    /*
	    fill `buf` with data from `blocknum`    
	*/
    assert(blocknum < NUM_DATA_BLOCKS);
    char tempBuf[BLOCKSIZE];
    lseek(DISK_FD, BLOCKSIZE * (5 + blocknum), SEEK_SET);
    memcpy(tempBuf, buf, BLOCKSIZE); 
    int ret = write(DISK_FD, tempBuf, BLOCKSIZE);
    assert(ret == BLOCKSIZE);
}

void simplefs_dump(){
    /*
	    Prints Disk state information   
	*/

    printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DISK STATE>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    struct superblock_t *superblock = (struct superblock_t *)malloc(sizeof(struct superblock_t));
    simplefs_readSuperBlock(superblock);
    char buf[MAX_NAME_STRLEN + 1];
    buf[MAX_NAME_STRLEN] = '\0';
    memcpy(buf, superblock->name, sizeof(buf) - 1);
    printf("DISK NAME: %s\nINODE FREELIST:\t", buf);
    for(int i=0; i<NUM_INODES; i++)
        printf("%c\t", superblock->inode_freelist[i]);
    printf("\nDATA BLOCK FREELIST:\t");
    for(int i=0; i<NUM_DATA_BLOCKS; i++)
        printf("%c\t", superblock->datablock_freelist[i]);
    printf("\n");

    struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
    for(int i=0; i<NUM_INODES; i++){
        simplefs_readInode(i, inode);
        if(inode->status == INODE_IN_USE){
            printf("INODE %d\nSTATUS:\t%c\tNAME\t%s\tSIZE\t%d\tDATABLOCK\t", i, inode->status, inode->name, inode->file_size);
            for (int j = 0; j < MAX_FILE_SIZE; j++)
                printf("%d\t", inode->direct_blocks[j]);
            printf("\n");
            for (int j = 0; j < MAX_FILE_SIZE; j++){
                if (inode->direct_blocks[j] != -1 ){
                    char tempBuf[BLOCKSIZE+1];
                    tempBuf[BLOCKSIZE] = '\0';
                    simplefs_readDataBlock(inode->direct_blocks[j], tempBuf);
                    printf("DATA BLOCK %d: %s\n", j, tempBuf);
                }
            }
            printf("\n");
        }     
    }
    free(inode);
    free(superblock);
    printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
}
