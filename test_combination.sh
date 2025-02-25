#!/bin/bash
set -e
set -x  # 启用命令打印

EMPTY_CHORUS_OPTS=("0" "1")
EMPTY_REVERB_OPTS=("0" "1")
GEN_TABLE_RUNTIME_OPTS=("0" "1")
ENABLE_7th_DSP_OPTS=("0" "1")

WITH_FLOAT_OPTS=("0" "1")
BUILD_OPTS=("Debug" "Release")
ARCH_OPTS=("x86_64" "i386" "arm")

# 遍历所有组合
for build in "${BUILD_OPTS[@]}"; do
    for arch in "${ARCH_OPTS[@]}"; do
        for with_float in "${WITH_FLOAT_OPTS[@]}"; do
            echo "Building with options: BUILD=$build, ARCH=$arch, WITH_FLOAT=$with_float"
            make BUILD=$build ARCH=$arch WITH_FLOAT=$with_float clean
            make BUILD=$build ARCH=$arch WITH_FLOAT=$with_float

            if [ "$arch"  !=  "arm" ]; then
                make BUILD=$build ARCH=$arch WITH_FLOAT=$with_float OPT=-O0 test
            fi
        done
    done
done
