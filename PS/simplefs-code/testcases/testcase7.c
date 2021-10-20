#include "simplefs-ops.h"
int main()
{
    char str[] = "!-----------------------64 Bytes of Data-----------------------!";
    simplefs_formatDisk();
    for (int i = 0; i < 8; i++)
    {
        char digit = i + '0';
        char fName[MAX_NAME_STRLEN];
        fName[MAX_NAME_STRLEN] = '\0';
        fName[0] = digit;
        strcpy(fName + 1, "_.txt");
        printf("Creating file %d_.txt: %d\n", i, simplefs_create(fName));
        int fd = simplefs_open(fName);
        char str[] = "!-----------------------64 Bytes of Data-----------------------!";
        printf("Writing Data: %d\n", simplefs_write(fd, str, BLOCKSIZE));
        simplefs_seek(fd, BLOCKSIZE);
        printf("Writing Data: %d\n", simplefs_write(fd, str, BLOCKSIZE));
        simplefs_seek(fd, BLOCKSIZE);
        printf("Writing Data: %d\n", simplefs_write(fd, str, BLOCKSIZE));
        simplefs_seek(fd, BLOCKSIZE);
        printf("Writing Data: %d\n", simplefs_write(fd, str, BLOCKSIZE));
        simplefs_seek(fd, BLOCKSIZE);
    }
    simplefs_dump();
}