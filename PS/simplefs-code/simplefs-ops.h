#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "simplefs-disk.h"

// Functions to implement in simplefs-ops.c
int simplefs_create(char *filename);
int simplefs_open(char *filename);
void simplefs_delete (char *filename); 
void simplefs_close(int file_handle);
int simplefs_read(int file_handle, char *buf, int nbytes);
int simplefs_write(int file_handle, char *buf, int nbytes);
int simplefs_seek(int file_handle, int nseek);