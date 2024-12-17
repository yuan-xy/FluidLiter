cmake -S . -B Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build Debug/
# cmake -S . -B Release -DCMAKE_BUILD_TYPE=Release
# cmake --build Release/

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


