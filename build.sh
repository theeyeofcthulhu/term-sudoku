#!/usr/bin/env bash

set -e

if [[ $1 == "clean" ]]; then
    rm -rf ./build
fi

mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Debug ..
make

cd ..
