#!/bin/bash
set -e

cmake -S . -B Debug -DCMAKE_BUILD_TYPE=Debug #-DWITH_FLOAT=0
cmake --build Debug/
# cmake -S . -B Release -DCMAKE_BUILD_TYPE=Release
# cmake --build Release/

rm instruments-test || true
gcc example/src/instruments.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o instruments-test
./instruments-test ./example/sf_/Boomwhacker.sf2

rm fluidlite-test || true
gcc example/src/main.c -g -Iinclude -IDebug -LDebug -lfluidlite -lm -o fluidlite-test
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


rm mono-test || true
gcc example/src/mono.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o mono-test
./mono-test ./example/sf_/Boomwhacker.sf2 mono.pcm
# ffmpeg -f s16le -ar 44100 -ac 1 -i mono.pcm mono.wav
# ffprobe -v quiet -print_format json -show_format -show_streams mono.wav

# gcc example/src/test2.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o test2
# valgrind --tool=massif --massif-out-file=massif_test2 ./test2


rm sfload_mem || true
gcc example/src/sfload_mem.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o sfload_mem
valgrind --tool=massif --massif-out-file=massif_sfload_mem ./sfload_mem mem.pcm
# massif-visualizer massif_sfload_mem

rm test3 || true
gcc example/src/test3.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o test3
./test3 example/sf_/GMGSx_1.sf2 test3.pcm
# ffmpeg -f s16le -ar 44100 -ac 1 -i test3.pcm test3.wav

gcc example/src/test4.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o test4
./test4 example/sf_/GMGSx_1.sf2 test4.pcm
# ffmpeg -f s16le -ar 44100 -ac 1 -i test4.pcm test4.wav

gcc example/src/misc.c -g -Iinclude -Isrc -IDebug -c
gcc example/src/test5.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm misc.o -o test5
./test5 example/sf_/GMGSx_1.sf2

gcc example/src/test_u8.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o test_u8
./test_u8 example/sf_/GMGSx_1.sf2

gcc example/src/test_song.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o test_song
./test_song
./test_song u12

# valgrind --dsymutil=yes --tool=callgrind --dump-instr=yes --collect-jumps=yes ./test_song u12
# export QT_SCALE_FACTOR=2
# kcachegrind callgrind.out.xx

gcc example/src/test_conv.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm misc.o -o test_conv
./test_conv

gcc example/src/test_vel.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm misc.o -o test_vel
./test_vel


gcc example/src/test_fpe.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm misc.o -o test_fpe
./test_fpe

gcc example/src/test_fpe2.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm misc.o -rdynamic -o test_fpe2
./test_fpe2

gcc example/src/test_vel2.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm misc.o -o test_vel2
./test_vel2

#gcc example/src/test_441.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o test_441
#./test_441