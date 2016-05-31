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

#ifndef _FUSE_PTFS_DEVICE_H_
#   define _FUSE_PTFS_DEVICE_H_

#   include <unistd.h>
#   include <parted/parted.h>
#   include <limits.h>

#   define FUSEPTFS_DEVICE_ERROR_OK   0
#   define FUSEPTFS_DEVICE_ERROR_FAIL 1

typedef struct FusePTFSPartition {
    int mIndex;
    int mNumber;
    off_t mStart;
    off_t mEnd;
    off_t mLength;
    char mFileName[PATH_MAX];
} FusePTFSPartition_t;

typedef struct FusePTFSDevice {
    FILE* mpFile;
    int mNumPartitions;
    FusePTFSPartition_t* mpPartitions;
} FusePTFSDevice_t;

int fuse_ptfs_device_init(FusePTFSDevice_t* apFusePTFSDevice, const char* apFileName, const char* apMountDir);
void fuse_ptfs_device_free(FusePTFSDevice_t* apFusePTFSDevice);
FusePTFSPartition_t* fuse_ptfs_device_find_partition(const char* apPath, FusePTFSDevice_t* apDevice);

#endif
