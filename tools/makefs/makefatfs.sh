#!/bin/bash

input=$1
output=$2

# 删除旧的文件
if [ -f $output ]; then
    rm $output
fi

# 制作文件系统[-S指定扇区大小(Byte)] [-C 文件 文件系统大小(KByte)]
mkfs.fat -n "Nor Flash" -F 12 -S 4096 -C $output 2800

# 创建临时文件夹
if [ ! -d "/tmp/makefatfs_tmp" ]; then
    mkdir /tmp/makefatfs_tmp
fi

# 挂载文件系统
sudo mount $output /tmp/makefatfs_tmp

# 复制二进制图片文件
sudo cp -rf $input /tmp/makefatfs_tmp

# 同步下文件
sync

# 卸载文件系统
sudo umount /tmp/makefatfs_tmp
rm -rf /tmp/makefatfs_tmp
