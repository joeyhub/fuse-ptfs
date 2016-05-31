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

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "fuse_ptfs_filesys.h"
#include "fuse_ptfs_device.h"
#include "fuse_ptfs_log.h"

void* fuse_ptfs_filesys_init(struct fuse_conn_info* apConn)
{
    LOG_ENT();

#ifdef _DEBUG
    extern FusePTFSDevice_t gFusePTFSDevice;
    LOG_FMT("&gFusePTFSDevice = 0x%x", &gFusePTFSDevice);
    LOG_FMT("fuse_get_context()->private_data = 0x%x", fuse_get_context()->private_data);
#endif

    return (FusePTFSDevice_t*)fuse_get_context()->private_data;
}

#ifdef _DEBUG
void fuse_ptfs_filesys_destroy(void* apUserData)
{
    LOG_ENT();

    extern FusePTFSDevice_t gFusePTFSDevice;
    LOG_FMT("&gFusePTFSDevice = 0x%x", &gFusePTFSDevice);
    LOG_FMT("fuse_get_context()->private_data = 0x%x", fuse_get_context()->private_data);
}
#endif

int fuse_ptfs_filesys_getattr(const char* apPath, struct stat* apStat)
{
    LOG_ENT();

    FusePTFSDevice_t* tpDevice = (FusePTFSDevice_t*)fuse_get_context()->private_data;
    // Note: This happens even if there is an error, could be moved our into a function for optimisation.
    memset(apStat, 0, sizeof(struct stat));
    apStat->st_uid = getuid();
    apStat->st_gid = getgid();
    // Note: Could also be current time or the time of the device. Currently this is redundent, given memset 0.
    apStat->st_atime = apStat->st_mtime = apStat->st_ctime = 0;

    if(0 == strcmp(apPath, "/"))
    {
        int tPartition = 0;

        apStat->st_mode = S_IFDIR | 0500;
        apStat->st_nlink = 2;

        // Note: Could be generated when the device is read on init.
        for(tPartition = 0; tPartition < tpDevice->mNumPartitions; ++tPartition)
            apStat->st_size += tpDevice->mpPartitions[tPartition].mLength;

        return 0;
    }

    FusePTFSPartition_t* tpPartition = fuse_ptfs_device_find_partition(apPath, tpDevice);

    if(NULL == tpPartition)
        return -ENOENT;

    // Note: I am unsure if fuse applies permissions with this or how to tie it into flags if need be.
    // Note: This does not consider the permissions of the underlying file.
    apStat->st_mode = S_IFREG | 0600;
    apStat->st_nlink = 1;
    apStat->st_size = tpPartition->mLength;

    return 0;
}

int fuse_ptfs_filesys_readdir(const char* apPath, void* apBuf, fuse_fill_dir_t aFiller, off_t aOffset, struct fuse_file_info* apFileInfo)
{
    LOG_ENT();

    if(0 != strcmp(apPath, "/"))
        return -ENOENT;

    FusePTFSDevice_t* tpDevice = (FusePTFSDevice_t*)fuse_get_context()->private_data;
    aFiller(apBuf, ".", NULL, 0);
    aFiller(apBuf, "..", NULL, 0);

    int tPartition = 0;

    for(tPartition = 0; tPartition < tpDevice->mNumPartitions; ++tPartition)
        // Note: It's not clear what the last two parameters are useful for (stat, off).
        // Warning: This pointer trickery is risky assuming #^/[^\0]+\0$# (regex), see fuse_ptfs_device.c and take care.
        aFiller(apBuf, &tpDevice->mpPartitions[tPartition].mFileName[1], NULL, 0);

    return 0;
}

int fuse_ptfs_filesys_open(const char* apPath, struct fuse_file_info* apFileInfo)
{
    LOG_ENT();

    FusePTFSDevice_t* tpDevice = (FusePTFSDevice_t*)fuse_get_context()->private_data;
    FusePTFSPartition_t* tpPartition = fuse_ptfs_device_find_partition(apPath, tpDevice);

    if(NULL == tpPartition)
        return -ENOENT;

    apFileInfo->fh = tpPartition->mIndex;
    return 0;
}

int fuse_ptfs_filesys_read(const char* apPath, char* apBuf, size_t aSize, off_t aOffset, struct fuse_file_info* apFileInfo)
{
    LOG_ENT();

    FusePTFSDevice_t* tpDevice = (FusePTFSDevice_t*)fuse_get_context()->private_data;
    FusePTFSPartition_t* tpPartition = &tpDevice->mpPartitions[apFileInfo->fh];
    size_t tLength = tpPartition->mLength;

    if(aOffset < tLength)
    {
        if(aOffset + aSize > tLength)
            aSize = tLength - aOffset;

        int tRetSeek = fseek(tpDevice->mpFile, tpPartition->mStart + aOffset, SEEK_SET);

        if(0 != tRetSeek)
        {
            LOG_FMT("Failed to set file position, for reading. Error = %d, %s", errno, strerror((errno)));
            return 0;
        }

        aSize = fread(apBuf, 1, aSize, tpDevice->mpFile);
        return aSize;
    }

    LOG_MSG("Offset beyond end of file.");
    return 0;
}

int fuse_ptfs_filesys_write(const char* apPath, const char* apBuf, size_t aSize, off_t aOffset, struct fuse_file_info* apFileInfo)
{
    LOG_ENT();

    FusePTFSDevice_t* tpDevice = (FusePTFSDevice_t*)fuse_get_context()->private_data;
    FusePTFSPartition_t* tpPartition = &tpDevice->mpPartitions[apFileInfo->fh];
    size_t tLength = tpPartition->mLength;

    if(aOffset < tLength)
    {
        if(aOffset + aSize > tLength)
            aSize = tLength - aOffset;

        int tRetSeek = fseek(tpDevice->mpFile, tpPartition->mStart + aOffset, SEEK_SET);

        if(0 != tRetSeek)
        {
            LOG_FMT("failed to set file position, for reading. Error = %d, %s", errno, strerror((errno)));
            return 0;
        }

        aSize = fwrite(apBuf, 1, aSize, tpDevice->mpFile);
        LOG_FMT("bytes written = %d", aSize);
        return aSize;
    }

    LOG_MSG("Offset beyond end of file.");
    return 0;
}

int fuse_ptfs_filesys_flush(const char* apPath, struct fuse_file_info* apFileInfo)
{
    LOG_ENT();

    FusePTFSDevice_t* tpDevice = (FusePTFSDevice_t*)fuse_get_context()->private_data;
    fflush(tpDevice->mpFile);
    return 0;
}

// Note: This is an anomaly with Linux and command interoperability.
int fuse_ptfs_filesys_truncate(const char* apPath, off_t aNewSize)
{
    LOG_ENT();

    return 0;
}
