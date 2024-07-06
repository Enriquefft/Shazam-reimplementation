if [ -d "build" ]; then
    echo "Removing build"
    rm -rf build
fi
cmake -S. -B build && cmake --build build
