#!/bin/bash
set -e

DEFAULT_BUILD="Debug"
BUILD=${1:-$DEFAULT_BUILD}
echo $BUILD

# sudo apt install gcc-multilib g++-multilib

# dpkg --add-architecture i386
# apt-get update
# apt-get install libc6-dbg:i386

#make clean BUILD=$BUILD
make -j BUILD=$BUILD


rm fluidlite-test || true
gcc example/src/main.c -m32 -DWITH_FLOAT -g -Iinclude -Isrc -I$BUILD -L$BUILD -lfluidlite -lm -o fluidlite-test
rm massif.out.* || true
valgrind --tool=massif ./fluidlite-test ./example/sf_/GMGSx_1.sf2 output.pcm
# ms_print massif.out.<pid>
# ffmpeg -f s16le -ar 44100 -ac 2 -i output.pcm output.wav
# ffmpeg -i output64.wav -filter:a volumedetect -f null /dev/null

# ffmpeg -i output64.wav -af astats -f null -
# 如果 Peak level 接近 0 dB，说明音频信号已经达到最大值，可能存在 clipping（削波）。
# 流行音乐通常会将 RMS(Root Mean Square) 电平控制在 -12 dBFS 到 -9 dBFS 之间
# dBFS（Decibels Full Scale，满量程分贝）, 对于 16-bit 音频，满量程值为32768, -9 dBFS对应的样本值11628

# 人耳舒适的音量范围是 60 dB 到 85 dB SPL, Sound Pressure Level（声压级）
# SPL 和 dBFS 之间没有直接的数学关系，因为 SPL 取决于播放设备的增益和环境的声学特性。


# valgrind --dsymutil=yes --tool=callgrind --dump-instr=yes --collect-jumps=yes ./test_song u12
# export QT_SCALE_FACTOR=2
# kcachegrind callgrind.out.xx

make clean
make C_DEFS="-DWITH_FLOAT  -DUSING_CALLOC=1 -DDEBUG=1"
make run_test_reverb_chorus  C_DEFS="-DWITH_FLOAT  -DUSING_CALLOC=1 -DDEBUG=1"
