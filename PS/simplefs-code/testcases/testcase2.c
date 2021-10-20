#include "simplefs-ops.h"

int main()
{
    simplefs_formatDisk();
    simplefs_create("fileabc");
    int fd = simplefs_open("fileabc");
    char str[] = "!-----------------------128 Bytes of Data----------------------!!-----------------------128 Bytes of Data----------------------!";
    printf("Write Data: %d\n", simplefs_write(fd, str, BLOCKSIZE*2));
    printf("Seek: %d\n", simplefs_seek(fd, BLOCKSIZE*2));
    printf("Write Data: %d\n", simplefs_write(fd, str, BLOCKSIZE*2));
    char buf[BLOCKSIZE*MAX_FILE_SIZE+1];
    buf[BLOCKSIZE*MAX_FILE_SIZE] = '\0';
    printf("Seek: %d\n", simplefs_seek(fd, -BLOCKSIZE*2));
    printf("Read Data %d\n", simplefs_read(fd, buf, BLOCKSIZE * MAX_FILE_SIZE));
    printf("Data: %s\n", buf);
    simplefs_close(fd);
    simplefs_dump();
}