#include "simplefs-ops.h"

int main()
{
    simplefs_formatDisk();
    simplefs_create("fileabc");
    int fd = simplefs_open("fileabc");
    char str1[] = "!-----------------------70 Bytes of Data-----------------------------!";
    char str2[] = "!-------------46 Bytes of Data---------------!";
    printf("Write Data: %d\n", simplefs_write(fd, str1, 70));
    printf("Seek: %d\n", simplefs_seek(fd, 70));
    printf("Write Data: %d\n", simplefs_write(fd, str1, 70));
    printf("Seek: %d\n", simplefs_seek(fd, 70));
    printf("Write Data: %d\n", simplefs_write(fd, str1, 70));
    printf("Seek: %d\n", simplefs_seek(fd, 70));
    printf("Write Data: %d\n", simplefs_write(fd, str2, 46));

    char buf1[71];
    buf1[70] = '\0';
    char buf2[47];
    buf2[46] = '\0';
    printf("Seek: %d\n", simplefs_seek(fd, -210));
    printf("Read Data %d\n", simplefs_read(fd, buf1, 70));
    printf("Data: %s\n", buf1);
    printf("Seek: %d\n", simplefs_seek(fd, 70));
    printf("Read Data %d\n", simplefs_read(fd, buf1, 70));
    printf("Data: %s\n", buf1);
    printf("Seek: %d\n", simplefs_seek(fd, 70));
    printf("Read Data %d\n", simplefs_read(fd, buf1, 70));
    printf("Data: %s\n", buf1);
    printf("Seek: %d\n", simplefs_seek(fd, 70));
    printf("Read Data %d\n", simplefs_read(fd, buf2, 46));
    printf("Data: %s\n", buf2);
    simplefs_dump();
}
