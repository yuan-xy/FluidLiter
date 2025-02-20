if [ $# -eq 0 ]; then
    make clean
    make -j
    make test
else
    if [ "$1" =  "all" ]; then
        rm -rf Debug/
        make -j
        make run_test

        make clean
        make EMPTY_CHORUS=1 EMPTY_REVERB=1 -j
        make test

        rm -rf Release
        make run_test BUILD=Release

        make ARCH=arm BUILD_DIR=build_arm
        make js BUILD_DIR=build_js
    fi

    if [ "$1" = "Release" ]; then
        rm -rf Release
        make run_test BUILD=Release
    fi
fi

