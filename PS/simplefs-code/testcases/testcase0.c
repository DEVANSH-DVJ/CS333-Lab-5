#include "simplefs-ops.h"

int main(){
  simplefs_formatDisk();
  simplefs_create("fileabc");
  int fd = simplefs_open("fileabc");
  char str[] = "!-----------------------64 Bytes of Data-----------------------!";
  printf("Write Data: %d\n", simplefs_write(fd, str, BLOCKSIZE));
  printf("Seek: %d\n", simplefs_seek(fd, BLOCKSIZE));
  printf("Write Data: %d\n", simplefs_write(fd, str, BLOCKSIZE));
  printf("Seek: %d\n", simplefs_seek(fd, BLOCKSIZE));
  char buf[BLOCKSIZE * 2 + 1];
  buf[BLOCKSIZE * 2] = '\0';
  printf("Seek: %d\n", simplefs_seek(fd, -BLOCKSIZE * 2));
  printf("Read Data %d\n", simplefs_read(fd, buf, BLOCKSIZE * 2));
  printf("Data: %s\n", buf);
  simplefs_close(fd);
  simplefs_dump();
}
