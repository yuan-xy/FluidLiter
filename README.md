# FluidLiter

[![License: LGPL-2.1](https://img.shields.io/badge/License-LGPL--2.1-brightgreen.svg)](https://opensource.org/licenses/LGPL-2.1)
[![Travis-CI Status](https://travis-ci.com/katyo/fluidlite.svg?branch=master)](https://travis-ci.com/katyo/fluidlite)


FluidLiter is a lighter version of FluidLite and is not compatible with FluidLite.

FluidLite (c) 2016 Robin Lobel is a very light version of FluidSynth
designed to be hardware, platform and external dependency independent.
It only uses standard C libraries.

FluidLite keeps very minimal functionalities (only synth),
therefore MIDI file reading, realtime MIDI events and audio output must be
implemented externally.

## Quick Start
To build the libfluidlite.a file:
~~~
$ cmake -S . -B Debug -DCMAKE_BUILD_TYPE=Debug
$ cmake --build Debug/

~~~

To build the example:
~~~
$ gcc example/src/main.c -g -Iinclude -IDebug -LDebug -lfluidlite -lm -o fluidlite-test
~~~
or
~~~
$ cd example/
$ cmake -S ./ -B ./Debug -DCMAKE_BUILD_TYPE=Debug
$ cmake --build Debug/
~~~

To run the example:
~~~
$ ./fluidlite-test <some soundfile> output.pcm
~~~

Maybe your media player can't play pcm file, so you can transform the pcm file to wav file:
~~~
$ ffmpeg -f s16le -ar 44100 -ac 2 -i output.pcm output.wav
~~~



## Configuration
To configure the sources with debug symbols:

~~~
$ cmake -S <source-directory> -B <build-directory> -DCMAKE_BUILD_TYPE=Debug
~~~

To configure the sources with an install prefix:

~~~
$ cmake -S <source-directory> -B <build-directory> -DCMAKE_INSTALL_PREFIX=/usr/local
---
$ cmake -S <source-directory> -B <build-directory> -DCMAKE_INSTALL_PREFIX=${HOME}/FluidLite
~~~

To configure the sources with additional dependency search paths:

~~~
$ cmake -S <source-directory> -B <build-directory> -DCMAKE_PREFIX_PATH=${HOME}/tests
~~~

To build (compile) the sources:

~~~
$ cmake --build <build-directory>
~~~

To install the compiled products (needs cmake 3.15 or newer):

~~~
$ cmake --install <build-directory>
~~~

Here is a bash script that you can customize/use on Linux:

~~~shell
#!/bin/bash
CMAKE="${HOME}/Qt/Tools/CMake/bin/cmake"
SRC="${HOME}/Projects/FluidLite"
BLD="${SRC}-build"
${CMAKE} -S ${SRC} -B ${BLD} \
    -DFLUIDLITE_BUILD_STATIC:BOOL="1" \
    -DFLUIDLITE_BUILD_SHARED:BOOL="1" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_SKIP_RPATH:BOOL="0" \
    -DCMAKE_INSTALL_LIBDIR="lib" \
    -DCMAKE_INSTALL_PREFIX=$HOME/FluidLite \
    -DCMAKE_VERBOSE_MAKEFILE:BOOL="1" \
    $*
read -p "Cancel now or pulse <Enter> to build"
${CMAKE} --build $BLD
read -p "Cancel now or pulse <Enter> to install"
${CMAKE} --install $BLD
~~~

See also the [complete cmake documentation](https://cmake.org/cmake/help/latest/manual/cmake.1.html) for more information.
