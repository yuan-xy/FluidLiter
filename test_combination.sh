#!/bin/bash
set -e
set -x  # 启用命令打印

EMPTY_CHORUS_OPTS=("0" "1")
EMPTY_REVERB_OPTS=("0" "1")
GEN_TABLE_RUNTIME_OPTS=("0" "1")
ENABLE_7th_DSP_OPTS=("0" "1")
SIMPLE_MEM_ALLOC_OPTS=("0" "1")

WITH_FLOAT_OPTS=("0" "1")
BUILD_OPTS=("Debug" "Release")
ARCH_OPTS=("x86_64" "i386" "arm")

# 遍历所有组合
for build in "${BUILD_OPTS[@]}"; do
    for arch in "${ARCH_OPTS[@]}"; do
        for with_float in "${WITH_FLOAT_OPTS[@]}"; do
        for eco in "${EMPTY_CHORUS_OPTS[@]}"; do
         for ero in "${EMPTY_REVERB_OPTS[@]}"; do
          for gtro in "${GEN_TABLE_RUNTIME_OPTS[@]}"; do
           for e7do in "${ENABLE_7th_DSP_OPTS[@]}"; do
           for malloc in "${SIMPLE_MEM_ALLOC_OPTS[@]}"; do
        
            echo "Building with options: BUILD=$build, ARCH=$arch, WITH_FLOAT=$with_float, EMPTY_CHORUS_OPTS=$eco EMPTY_REVERB_OPTS=$ero GEN_TABLE_RUNTIME_OPTS=$gtro ENABLE_7th_DSP_OPTS=$e7do SIMPLE_MEM_ALLOC=$malloc"

            make BUILD=$build ARCH=$arch clean

            make BUILD=$build ARCH=$arch WITH_FLOAT=$with_float \
             EMPTY_CHORUS_OPTS=$eco EMPTY_REVERB_OPTS=$ero \
             GEN_TABLE_RUNTIME_OPTS=$gtro ENABLE_7th_DSP_OPTS=$e7do \
             SIMPLE_MEM_ALLOC=$malloc DEFAULT_LOG_LEVEL=2

            if [ "$arch"  !=  "arm" ]; then
                make BUILD=$build ARCH=$arch WITH_FLOAT=$with_float EMPTY_CHORUS_OPTS=$eco EMPTY_REVERB_OPTS=$ero GEN_TABLE_RUNTIME_OPTS=$gtro ENABLE_7th_DSP_OPTS=$e7do SIMPLE_MEM_ALLOC=$malloc OPT=-O0 test
            fi
            done
            done
           done
          done
         done
        done
    done
done
