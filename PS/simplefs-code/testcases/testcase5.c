#include "simplefs-ops.h"

int main()
{
    char str[] = "!-----------------------64 Bytes of Data-----------------------!";
    simplefs_formatDisk();

    simplefs_create("f1.txt");
    int fd1 = simplefs_open("f1.txt");
    simplefs_create("f2.txt");
    int fd2 = simplefs_open("f2.txt");

    printf("Write Data: %d\n", simplefs_write(fd1, str, BLOCKSIZE));
    printf("Write Data: %d\n", simplefs_write(fd2, str, BLOCKSIZE));
    printf("Seek: %d\n", simplefs_seek(fd1, BLOCKSIZE));
    printf("Seek: %d\n", simplefs_seek(fd2, BLOCKSIZE));
    printf("Write Data: %d\n", simplefs_write(fd1, str, BLOCKSIZE));
    printf("Write Data: %d\n", simplefs_write(fd2, str, BLOCKSIZE));
    simplefs_dump();
    simplefs_delete("f2.txt");
    simplefs_dump();
    simplefs_delete("f1.txt");
    simplefs_dump();
}