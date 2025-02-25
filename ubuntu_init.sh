#!/bin/bash
set -e
set -x  # 启用命令打印


sudo apt-get update
sudo apt install build-essential libc6-dev ffmpeg
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt install gcc-multilib g++-multilib libc6-dbg:i386

