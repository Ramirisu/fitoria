#!/bin/sh

cmake \
  -B build \
  -G Ninja \
  -DCMAKE_CXX_COMPILER=/usr/bin/g++-13 \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DFITORIA_BUILD_TESTS=ON \
  -DFITORIA_ENABLE_CODECOV=ON \
  -DCMAKE_TOOLCHAIN_FILE=${VCPKG_INSTALLATION_ROOT}/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_MANIFEST_FEATURES="tls;zlib;brotli;liburing;test"

cmake --build build -j2

cd build && ctest && cd ..

mkdir .coverage
lcov --gcov-tool=gcov-13 --capture --directory $(pwd)'/build' --output-file ./coverage/coverage.info
lcov --gcov-tool=gcov-13 --extract ./coverage/coverage.info $(pwd)'/include/fitoria/*' --output-file ./coverage/coverage.info

genhtml -o ./coverage/html --legend --title "fitoria" --prefix=./ ./coverage/coverage.info
