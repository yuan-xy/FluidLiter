cmake -S . -B Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build Debug/
gcc example/src/main.c -g -Iinclude -IDebug -LDebug -lfluidlite -lm -o fluidlite-test
valgrind --tool=massif ./fluidlite-test /mnt/c/tools/fluidsynth/bin/GMGSx.sf2 output.pcm