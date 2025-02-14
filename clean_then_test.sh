if [ $# -eq 0 ]; then
    rm -rf Debug/
    ./example_tests.sh
else
    if [ "$1" =  "all" ]; then
        rm -rf Debug/
        ./example_tests.sh

        rm -rf Release
        ./example_tests.sh Release

        make ARCH=arm BUILD_DIR=build_arm
        make js BUILD_DIR=build_js
    fi

    if [ "$1" = "Release" ]; then
        rm -rf Release
        ./example_tests.sh Release
    fi
fi

