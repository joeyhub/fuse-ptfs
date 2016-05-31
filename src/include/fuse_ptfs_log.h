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

#ifndef _FUSE_PTFS_LOG_H_
#   define _FUSE_PTFS_LOG_H_

#   ifdef _DEBUG
#       include <stdlib.h>
#       include <stdio.h>

#       define LOG_FILE_NAME "fuse_ptfs.log"
#       define LOG_FILE gpLogFile
#       define LOG_MSG(msg)      { fprintf(LOG_FILE, "%s(): " msg "\n", __FUNCTION__); LOG_FLUSH(); }
#       define LOG_FMT(fmt, ...) { fprintf(LOG_FILE, "%s(): " fmt "\n", __FUNCTION__, ##__VA_ARGS__); LOG_FLUSH(); }
#       define LOG_ENT()         { fprintf(LOG_FILE, "%s(): entered.\n", __FUNCTION__); LOG_FLUSH(); }
#       define LOG_HIT()         { fprintf(LOG_FILE, "%s(): %s\n", __FUNCTION__, __LINE__); LOG_FLUSH(); }
#       define LOG_FLUSH()     fflush(LOG_FILE)

extern FILE* gpLogFile;

void fuse_ptfs_log_init();
void fuse_ptfs_log_free();

#   else
#       define LOG_MSG(msg)
#       define LOG_FMT(fmt, ...)
#       define LOG_ENT()
#       define LOG_HIT()
#   endif
#endif
