#!/bin/bash
set -e
set -x  # 启用命令打印

if [ $# -eq 0 ]; then
    make clean
    make -j V=1
    make test
else
    if [ "$1" =  "all" ]; then
        rm -rf Debug/
        make -j
        make run_test -j

        make clean
        make EMPTY_CHORUS=1 EMPTY_REVERB=1 -j
        make test -j

        make clean
        make EMPTY_CHORUS=1 EMPTY_REVERB=1 GEN_TABLE_RUNTIME=1 -j
        make test -j

        rm -rf Release
        make run_test BUILD=Release -j

        rm -rf build_arm
        make ARCH=arm BUILD_DIR=build_arm -j

        rm -rf build_js
        make js BUILD_DIR=build_js -j
    fi

    if [ "$1" = "Release" ]; then
        rm -rf Release
        make run_test BUILD=Release -j
    fi

fi

