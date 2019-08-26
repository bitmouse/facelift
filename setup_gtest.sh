#! /bin/bash

THIS_DIR=$PWD

git clone https://github.com/google/googletest.git
cd googletest
mkdir -p build
mkdir -p install
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$PWD/../install
export CMAKE_SYSTEM_PREFIX_PATH=$PWD/../install
make
make install
cd ${THIS_DIR}
