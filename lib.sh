#!/bin/bash

cd "$(dirname $0)"

do_gcc () {
	FILE_IN="$1"
	FILE_OUT="$2"
	FLAGS="$3"

	echo GCC: "$FILE_IN"
	gcc -c "$FILE_IN" $FLAGS -std=gnu99 -pedantic -Wall -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -o "$FILE_OUT" -I./include
}

do_build () {
	FLAGS="$1"
	NAME="$2"
	EXTRA="$3"

	[ -f "$NAME" ] && rm -f "$NAME"

	if compgen -G "*.o" > /dev/null
	then
		rm *.o
	fi

	for FILE in '' _device _filesys $EXTRA
	do
		do_gcc fuse_ptfs${FILE}.{c,o} "$FLAGS"
	done

	echo G++: "$NAME"
	g++ *.o -o "$NAME" -L. -lparted -lfuse
}
