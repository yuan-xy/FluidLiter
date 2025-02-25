#!/bin/bash
set -e
set -x  # 启用命令打印


sudo apt-get update
sudo apt install build-essential libc6-dev ffmpeg
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt install gcc-multilib g++-multilib libc6-dbg:i386

sudo apt install cmake default-jre git python3
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
emcc --version

make js BUILD_DIR=build_js -j #test wasm


sudo apt-get install gcc-arm-none-eabi
arm-none-eabi-gcc -v
