#!/bin/bash

if [ "$CXX" = "g++" ]; then cmake -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DSDL2=OFF ../; fi

if [ "$CXX" = "clang++" ]; then cmake -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DSDL2=OFF ../; fi

if [ "$CXX" = "mingw" ]; then cmake -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../scripts/mingw32.toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DSDL2=OFF -DBUNDLED_FREETYPE=ON -DBUNDLED_OPENAL=ON ../ ; fi
