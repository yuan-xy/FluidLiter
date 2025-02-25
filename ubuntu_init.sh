#!/bin/bash
set -e
set -x  # 启用命令打印


apt-get update
apt install build-essential libc6-dev
dpkg --add-architecture i386
apt install gcc-multilib g++-multilib libc6-dbg:i386

