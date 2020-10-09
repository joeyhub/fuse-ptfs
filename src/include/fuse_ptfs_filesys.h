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

#ifndef _FUSE_PTFS_FILESYS_H_
#   define _FUSE_PTFS_FILESYS_H_


#ifndef FUSE_USE_VERSION
#   define FUSE_USE_VERSION 39
#endif
#   include <fuse.h>
#   include <stdio.h>
#   include <string.h>
#   include <errno.h>
#   include <fcntl.h>

enum{
    FUSEPTFS_FILESYS_ERROR_OK = 0,
    FUSEPTFS_FILESYS_ERROR_FAIL = 1
};

void* fuse_ptfs_filesys_init(struct fuse_conn_info* conn
#if FUSE_MAJOR_VERSION >= 3
    , struct fuse_config *
#endif
);

#   ifdef _DEBUG
void fuse_ptfs_filesys_destroy(void *userdata);
#   endif

int fuse_ptfs_filesys_getattr(const char*, struct stat*
#if FUSE_MAJOR_VERSION >= 3
    , struct fuse_file_info *
#endif
);

int fuse_ptfs_filesys_readdir(const char*, void*, fuse_fill_dir_t, off_t, struct fuse_file_info*
#if FUSE_MAJOR_VERSION >= 3
    , enum fuse_readdir_flags
#endif
);

int fuse_ptfs_filesys_open(const char*, struct fuse_file_info*);
int fuse_ptfs_filesys_read(const char*, char*, size_t, off_t, struct fuse_file_info*);
int fuse_ptfs_filesys_write(const char*, const char*, size_t, off_t, struct fuse_file_info*);

int fuse_ptfs_filesys_truncate(const char*, off_t
#if FUSE_MAJOR_VERSION >= 3
    , struct fuse_file_info *
#endif
);

int fuse_ptfs_filesys_flush(const char*, struct fuse_file_info*);

#endif
