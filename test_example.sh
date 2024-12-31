cmake -S . -B Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build Debug/
# cmake -S . -B Release -DCMAKE_BUILD_TYPE=Release
# cmake --build Release/

rm instruments-test
gcc example/src/instruments.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o instruments-test
./instruments-test ./example/sf_/Boomwhacker.sf2

rm fluidlite-test
gcc example/src/main.c -g -Iinclude -IDebug -LDebug -lfluidlite -lm -o fluidlite-test
rm massif.out.*
valgrind --tool=massif ./fluidlite-test ./example/sf_/Boomwhacker.sf2 output.pcm
# ms_print massif.out.<pid>

rm mono-test
gcc example/src/mono.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o mono-test
./mono-test ./example/sf_/Boomwhacker.sf2 mono.pcm
# ffmpeg -f s16le -ar 44100 -ac 1 -i mono.pcm mono.wav
# ffprobe -v quiet -print_format json -show_format -show_streams mono.wav

# gcc example/src/test2.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o test2
# valgrind --tool=massif --massif-out-file=massif_test2 ./test2


rm sfload_mem
gcc example/src/sfload_mem.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o sfload_mem
valgrind --tool=massif --massif-out-file=massif_sfload_mem ./sfload_mem mem.pcm
# massif-visualizer massif_sfload_mem

rm test3
gcc example/src/test3.c -g -Iinclude -Isrc -IDebug -LDebug -lfluidlite -lm -o test3
./test3 example/sf_/GMGSx_1.sf2 test3.pcm
# ffmpeg -f s16le -ar 44100 -ac 1 -i test3.pcm test3.wav
