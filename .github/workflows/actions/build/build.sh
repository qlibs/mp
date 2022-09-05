#!bin/sh

cd /github/home/mp
git submodule update --init

mkdir build-$1
cd build-$1
CXX=$1 cmake .. -DBOOST_MP_BUILD_TESTS=ON -DBOOST_MP_BUILD_EXAMPLES=ON
make -j8
ctest --output-on-failure
