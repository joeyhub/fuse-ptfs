Warning
=======

This is an early release, use at your own risk. Software has no warranty.

It is very likely to be unstable in certain situations especially where concurrency is needed.

It is recommended to only use it on a testing basis unless you know what you are doing and can test for consistency after making file operations.

Introduction
============

fuse-ptfs is a fuse program for mounting and writing to partitions using libparted to enumerate the partitions.

The current implementation is very minimal. For suggestions on contributions to make it more complete please see the contributions section.

It is hoped that this module will grow and improve over time eventually making it into major distributions.

Usage
=====

To use the program simply run it as so:

    fuse-ptfs [block device or file] [mount point]

This program does not support changes to the partition table while mounted and has not been tested for concurrency.

Purpose
=======

In Linux mounting filesystems recursively is simple while there are no common facilities to mount partition tables in a simple way. This is particularly problematic when building regular files that are to be later copied to a block device. Creating a partition table and file systems to be transferred to block devices is a common task that this program can help to simplify.

Currently there are methods for dealing with this which work but are more inconvenient and obtrusive than need be. Recommended reading to get an overview:

* https://www.suse.com/communities/blog/accessing-file-systems-disk-block-image-files/
* https://en.opensuse.org/SDB:Basics_of_partitions,_filesystems,_mount_points
* http://www.tldp.org/HOWTO/Partition/devices.html

Simple example of traditional approach with losetup (do not run, not fully tested):

    dd if=/dev/zero of=hdd.img bs=8192 count=256
    parted -s hdd.img mklabel gpt
    parted -sa optimal hdd.img mkpart primary ext3 0% 100%
    mknod block b 7 500
    echo losetup -o $(parted hdd.img unit B print | tail -n2 | head -n1 | sed -r 's/^ 1 +([0-9]+)B.*$/\1/') block hdd.img
    mkfs.ext3 block
    losetup -d block

This is fairly unobtrusive but has a quirk that the minor number for mknod is used to identify the block device, not the path so you would have to manage that. Extracting the partition offset is also inelegant. There are other approaches but they all tend to have various quirks that makes them unpleasant to use These can either be side effects or inconveniences such as needing to rely on crude text extractions and script wrapper. kpartx is a nice wrapper for this but also has side effects.

With this library you can do:

    dd if=/dev/zero of=hdd.img bs=8192 count=256
    parted -s hdd.img mklabel gpt
    parted -sa optimal hdd.img mkpart primary ext3 0% 100%
    mkdir partitions
    fuse-ptfs hdd.img partitions
    mkfs.ext3 partitions/1
    umount partitions

Design Choices
==============

The partition table is to be read once on program initialisation as a design choice. This can provide faster operations later and some consistent behaviour if the partition table is alterned while mounted (although it should not be).

It should not be possible to modify the partition table using this library. Instead it is intended to make changes to the partitions themselves that the partition table presents.

Dependencies
============

libfuse
libparted

Contributions
=============

Contributions may be made as is normal for projects on github.

If anyone would like to make a contribution please see the suggestions below. These also give an indication of the status of the program, what pull requests are likely to be accepted and serve as a vague roadmap.

Priorities:

1. Stability.
2. Build, Testing and Packaging.
3. Optimisation and Portability.

Suggestions:

* Review on autoconf/automake.
* Dependencies including versions are not fully listed.
* Packaging for debian/centos or other major distributions included.
* Concurrency safety has not been examined for any scenarios.
* In memory partion table should be left as run through unless someone can provide a good reason for optimising it.
* The use of PATH_MAX wastes a fair bit of memory per in memory-partition however the same applies as the above point.
* There is no proper fuse integration for fuse arguments.
* Multi partition table drivers could be worth considering if there is a problem with parted. For example other libraries might be more commonly found on systems by default.
* Creating or fixing issues is always welcome.
* Appropriate standardisation is welcome.
* Optimisations are welcome where appropriate as long as there is a good trade off.
* There is currently no code documentation. doxygen format and Doxyfile would be accepted.
* There are no automated tests for building.
* There is no make install (including symlink for mount command).
* The current debug options (CPP) are peculiar.
* No support for guid, etc. If this is added it is best added as a flag or option rather than mounting to provide many means of access.
* Single filehandle used raises questions on portability.
* Portability improvements are welcome.
* Working out where it is better to use parted's block device functions or libc functions for file access.

Credits
=======

Another author who wishes to remain anonymous contributed most of the original code to this module after been assigned this as a task.

A similar program exists which was not found until after this implementation was created:
https://github.com/madscientist42/partitionfs
