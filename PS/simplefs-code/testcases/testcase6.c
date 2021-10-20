#include "simplefs-ops.h"

int main()
{
    char str[] = "!-----------------------64 Bytes of Data-----------------------!";
    simplefs_formatDisk();

    for (int i = 0; i < 10; i++)
    {
        char digit = i + '0';
        char fName[MAX_NAME_STRLEN];
        fName[MAX_NAME_STRLEN] = '\0';
        fName[0] = digit;
        strcpy(fName + 1, "_.txt");
        printf("Creating file %d_.txt: %d\n", i, simplefs_create(fName));
    }
    simplefs_dump();
}