#!/bin/bash

# parameter 1: directory of files to be copied
# parameter 2: output file path of the file system
# parameter 3: size of the file system

input=$1
output=$2
fssize=$3

# delete the old file system
if [ -f $output ]; then
    rm $output
fi

# making a file system [-S specifies sector size (Byte)] [-C output file path and file system size (KByte)]
mkfs.fat -n "Nor Flash" -F 12 -S 4096 -C $output $fssize

# create a temporary folder
if [ ! -d "/tmp/makefatfs_tmp" ]; then
    mkdir /tmp/makefatfs_tmp
fi

# mount the file system
mount $output /tmp/makefatfs_tmp

# copy files to a temporary folder
cp -rf $input/* /tmp/makefatfs_tmp

# ensure that the file is written to the file system
sync

# unmount the file system
umount /tmp/makefatfs_tmp

# delete temporary folders
rm -rf /tmp/makefatfs_tmp
