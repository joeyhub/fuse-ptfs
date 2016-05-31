/**
 * fuse-ptfs: Allows mounting partition tables via fuse.
 * Copyright (C) 2016 Joey (https://github.com/joeyhub)
 *
 * This file is part of fuse-ptfs.
 *
 * fuse-ptfs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fuse-ptfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fuse-ptfs. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/limits.h>

#include <parted/parted.h>
#ifndef FUSE_USE_VERSION
#   define FUSE_USE_VERSION 26
#endif
#include <fuse.h>

#include "fuse_ptfs_device.h"
#include "fuse_ptfs_filesys.h"
#include "fuse_ptfs_log.h"

#define FUSEPTFS_ERROR_OK   0
#define FUSEPTFS_ERROR_FAIL 1

FusePTFSDevice_t gFusePTFSDevice;
char gDeviceFileName[PATH_MAX];
char gMountDir[PATH_MAX];
char gCurrentDir[PATH_MAX];

struct fuse_operations gFusePTFSOperations = {
    .init     = fuse_ptfs_filesys_init,
#ifdef _DEBUG
    .destroy  = fuse_ptfs_filesys_destroy,
#endif
    .getattr  = fuse_ptfs_filesys_getattr,
    .readdir  = fuse_ptfs_filesys_readdir,
    .open     = fuse_ptfs_filesys_open,
    .read     = fuse_ptfs_filesys_read,
    .write    = fuse_ptfs_filesys_write,
    .truncate = fuse_ptfs_filesys_truncate,
    .flush    = fuse_ptfs_filesys_flush
};

void print_usage(void)
{
    const char* tpUsage = "Usage: fuse_ptfs device dir";
    printf(tpUsage);
}

int main(int argc, char* argv[])
{
    if(3 != argc)
    {
        printf("Not enough arguments.\n");
        print_usage();
        printf("\n");
        return FUSEPTFS_ERROR_FAIL;
    }

    const char* tpDeviceFileName = argv[1];
    const char* tpMountDir = argv[2];

    if(0 != access(tpDeviceFileName, F_OK))
    {
        printf("file: %s, does not exist\n", tpDeviceFileName);
        return FUSEPTFS_ERROR_FAIL;
    }

    struct stat tDeviceFileStat;

    if(0 == stat(tpDeviceFileName, &tDeviceFileStat))
    {
        if(!S_ISREG(tDeviceFileStat.st_mode) && !S_ISBLK(tDeviceFileStat.st_mode))
        {
            printf("file: %s, is not a valid regular file or block device\n", tpDeviceFileName);
            return FUSEPTFS_ERROR_FAIL;
        }

        printf("file: %s, is valid.\n", tpDeviceFileName);
    }
    else
    {
        printf("unable to verify type of file: %s\n", tpDeviceFileName);
        return FUSEPTFS_ERROR_FAIL;
    }

    if(0 != access(tpMountDir, F_OK))
    {
        printf("Mount directory: %s, does not exist\n", tpMountDir);
        return FUSEPTFS_ERROR_FAIL;
    }

    struct stat tMountDirStat;

    if(0 == stat(tpMountDir, &tMountDirStat))
    {
        if(!S_ISDIR(tMountDirStat.st_mode))
        {
            printf("Mount directory: %s, is not a directory\n", tpMountDir);
            return FUSEPTFS_ERROR_FAIL;
        }

        printf("Mount directory: %s, is valid.\n", tpMountDir);
    }
    else
    {
        printf("unable to verify mount directory: %s\n", tpMountDir);
        return FUSEPTFS_ERROR_FAIL;
    }

#ifdef _DEBUG
    fuse_ptfs_log_init();
#endif

    LOG_MSG("fuse_ptfs, start logging ...");
    LOG_FMT("file: %s", tpDeviceFileName);
    LOG_FMT("mount dir: %s", tpMountDir);
    printf("device file: %s\n", tpDeviceFileName);
    printf("mount directory: %s\n", tpMountDir);

    int tDeviceError = fuse_ptfs_device_init(&gFusePTFSDevice, tpDeviceFileName, tpMountDir);

    if(FUSEPTFS_DEVICE_ERROR_OK != tDeviceError)
    {
        printf("failed to initialise file data.\n");
        return FUSEPTFS_ERROR_FAIL;
    }

    printf("Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);
    LOG_FMT("Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);

#ifdef _DEBUG
    char* tArgV[] = {argv[0], "-f", "-d", "-s", argv[2]};
    int tArgC = 5;
#else
    char* tArgV[] = {argv[0], "-s", argv[2]};
    int tArgC = 3;
#endif

    LOG_MSG("calling fuse_main()");
    printf("mounting ...\n");

    int tFuseError = fuse_main(tArgC, tArgV, &gFusePTFSOperations, &gFusePTFSDevice);

    if(FUSEPTFS_FILESYS_ERROR_OK != tFuseError)
        return FUSEPTFS_ERROR_FAIL;

    fuse_ptfs_device_free(&gFusePTFSDevice);
#ifdef _DEBUG
    fuse_ptfs_log_free();
#endif

    return FUSEPTFS_ERROR_OK;
}
