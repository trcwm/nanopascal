#!/bin/sh

mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
cd ..
