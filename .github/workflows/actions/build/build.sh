#!bin/sh

cd /github/home/mp
git submodule update --init

mkdir build
cd build
CXX=$1 cmake .. -DBOOST_MP_BUILD_TESTS=ON -DBOOST_MP_BUILD_EXAMPLES=ON
make -j8
ctest --output-on-failure
