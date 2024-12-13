cmake -S . -B Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build Debug/
# cmake -S . -B Release -DCMAKE_BUILD_TYPE=Release
# cmake --build Release/
gcc example/src/main.c -g -Iinclude -IDebug -LDebug -lfluidlite -lm -o fluidlite-test
valgrind --tool=massif ./fluidlite-test ./example/sf_/Boomwhacker.sf2 output.pcm
# ms_print massif.out.<pid>
