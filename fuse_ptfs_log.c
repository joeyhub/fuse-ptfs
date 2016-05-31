#include "fuse_ptfs_log.h"

#ifdef _DEBUG

FILE* gpLogFile = NULL;

void fuse_ptfs_log_init()
{
    gpLogFile = fopen(LOG_FILE_NAME, "w");
    if(!gpLogFile)
        printf("Failed to open logfile\n");
}

void fuse_ptfs_log_free()
{
    fclose(gpLogFile);
}

#endif
