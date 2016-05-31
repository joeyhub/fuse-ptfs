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

#include <parted/parted.h>

#include "fuse_ptfs_device.h"
#include "fuse_ptfs_log.h"

int fuse_ptfs_device_init(FusePTFSDevice_t* apFusePTFSDevice, const char* apFileName, const char* apMountDir)
{
    if(NULL == apFusePTFSDevice || NULL == apFileName || NULL == apMountDir)
        return FUSEPTFS_DEVICE_ERROR_FAIL;

    int tNumPartitions = 0;

    PedDevice* tpDevice = ped_device_get(apFileName);

    if(NULL == tpDevice)
        return FUSEPTFS_DEVICE_ERROR_FAIL;

    // Explain: This is actually representative of the partition table (disk label).
    PedDisk* tpDisk = ped_disk_new(tpDevice);

    if(NULL == tpDisk)
    {
        ped_device_destroy(tpDevice);
        return FUSEPTFS_DEVICE_ERROR_FAIL;
    }

    PedPartition* tpPartition = ped_disk_next_partition(tpDisk, NULL);

    while(NULL != tpPartition)
    {
        // Note: We do no sanity checking on partitions, I assume this adhere's to libparted's sanity checking.
        if(tpPartition->num >= 0)
            ++tNumPartitions;

        tpPartition = ped_disk_next_partition(tpDisk, tpPartition);
    }

    if(0 == tNumPartitions)
    {
        LOG_MSG("Partition table is empty.")
        ped_disk_destroy(tpDisk);
        ped_device_destroy(tpDevice);
        return FUSEPTFS_DEVICE_ERROR_FAIL;
    }

    apFusePTFSDevice->mpFile = fopen(apFileName, "rb+");

    if(NULL == apFusePTFSDevice->mpFile)
    {
        LOG_MSG("Failed to open file for reading and writing.")
        ped_disk_destroy(tpDisk);
        ped_device_destroy(tpDevice);
        return FUSEPTFS_DEVICE_ERROR_FAIL;
    }

    rewind(apFusePTFSDevice->mpFile);

    apFusePTFSDevice->mNumPartitions = tNumPartitions;
    // Note: Missing check here for failure.
    apFusePTFSDevice->mpPartitions = (FusePTFSPartition_t*)malloc(tNumPartitions * sizeof(FusePTFSPartition_t));
    memset(apFusePTFSDevice->mpPartitions, 0, tNumPartitions * sizeof(FusePTFSPartition_t));

    printf("Partitions:\n");
    printf("%3s\t%11s\t%11s\t%11s\t%s\n", "no.", "start", "end", "size", "fs");

    int tIndex = 0;

    tpPartition = ped_disk_next_partition(tpDisk, NULL);

    while(NULL != tpPartition)
    {
        if(tpPartition->num < 0)
        {
            tpPartition = ped_disk_next_partition(tpDisk, tpPartition);
            continue;
        }

        printf(
            "%3d\t%10lldB\t%10lldB\t%10lldB\t%s\n",
            tpPartition->num,
            tpPartition->geom.start * tpDevice->sector_size,
            (tpPartition->geom.end + 1) * tpDevice->sector_size - 1,
            tpPartition->geom.length * tpDevice->sector_size,
            tpPartition->fs_type ? tpPartition->fs_type->name : ""
        );

        apFusePTFSDevice->mpPartitions[tIndex].mIndex = tIndex;
        apFusePTFSDevice->mpPartitions[tIndex].mNumber = tpPartition->num;
        apFusePTFSDevice->mpPartitions[tIndex].mStart  = tpPartition->geom.start * tpDevice->sector_size;
        apFusePTFSDevice->mpPartitions[tIndex].mEnd    = (tpPartition->geom.end + 1) * tpDevice->sector_size - 1;
        apFusePTFSDevice->mpPartitions[tIndex].mLength = tpPartition->geom.length * tpDevice->sector_size;
        sprintf(apFusePTFSDevice->mpPartitions[tIndex].mFileName, "/%d", tpPartition->num);

        ++tIndex;
        tpPartition = ped_disk_next_partition(tpDisk, tpPartition);
    }

    ped_disk_destroy(tpDisk);
    ped_device_destroy(tpDevice);

    return FUSEPTFS_DEVICE_ERROR_OK;
}

void fuse_ptfs_device_free(FusePTFSDevice_t* apFusePTFSDevice)
{
    if(NULL == apFusePTFSDevice)
        return;

    if(NULL != apFusePTFSDevice->mpFile)
    {
        fclose(apFusePTFSDevice->mpFile);
        apFusePTFSDevice->mpFile = NULL;
    }

    free(apFusePTFSDevice->mpPartitions);
    apFusePTFSDevice->mpPartitions = NULL;
}

FusePTFSPartition_t* fuse_ptfs_device_find_partition(const char* apPath, FusePTFSDevice_t* apDevice)
{
    int tPartition = 0;
    FusePTFSPartition_t* tpPartitions = apDevice->mpPartitions;

    for(tPartition = 0; tPartition < apDevice->mNumPartitions; ++tPartition)
        if(0 == strcmp(apPath, tpPartitions[tPartition].mFileName))
            return &tpPartitions[tPartition];

    return NULL;
}
