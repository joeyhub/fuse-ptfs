AM_CFLAGS=-O2 -std=gnu99 -pedantic -Wall -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -I./include
AM_LDFLAGS=

bin_PROGRAMS=fuse-ptfs
fuse_ptfs_SOURCES=fuse_ptfs.c fuse_ptfs_device.c fuse_ptfs_filesys.c
fuse_ptfs_LDFLAGS=-lfuse -lparted
