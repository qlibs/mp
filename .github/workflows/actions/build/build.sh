#!bin/bash

cd /mp

mkdir build-clang
cd build-clang
CXX=clang++-16 cmake .. -DBOOST_MP_BUILD_TESTS=ON -DBOOST_MP_BUILD_EXAMPLES=ON
make -j8
ctest --output-on-failure

cd ..

mkdir build-gcc
cd build-gcc
CXX=clang++-16 cmake .. -DBOOST_MP_BUILD_TESTS=ON -DBOOST_MP_BUILD_EXAMPLES=ON
make -j8
ctest --output-on-failure
