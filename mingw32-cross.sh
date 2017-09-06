#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cmake -DCMAKE_TOOLCHAIN_FILE=$DIR/mingw32-cross.cmake $DIR
