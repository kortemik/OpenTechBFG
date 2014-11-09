cd ..
mkdir -p build
cd build

cmake -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../neo/mingw32.toolchain.cmake ../neo
