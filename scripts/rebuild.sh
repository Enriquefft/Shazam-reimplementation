if [ -d "build" ]; then
    rm -rf build
fi
cmake -S. -B build && cmake --build build
