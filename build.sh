export CXX=g++
export CC=gcc
rm -rf build && cmake -S . -B build && cmake --build build
