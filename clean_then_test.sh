if [ $# -eq 0 ]; then
    rm -rf Debug/
    ./example_tests.sh
else
    if [ "$1" =  "all" ]; then
        rm -rf Debug/
        ./example_tests.sh
        rm -rf Release
        ./example_tests.sh Release
    fi

    if [ "$1" = "Release" ]; then
        rm -rf Release
        ./example_tests.sh Release
    fi
fi

