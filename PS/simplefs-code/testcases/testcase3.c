#include "simplefs-ops.h"

int main()
{
    simplefs_formatDisk();
    simplefs_create("fileabc");
    int fd = simplefs_open("fileabc");
    simplefs_close(fd);
    simplefs_dump();
    simplefs_delete("fileabc");
    simplefs_dump();
}
