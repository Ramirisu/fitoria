# Configure
cmake -B build \
    -DCMAKE_CXX_COMPILER=/usr/bin/g++-12 \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DFITORIA_BUILD_TESTS=ON \
    -DFITORIA_ENABLE_CODECOV=ON \
    -DCPM_SOURCE_CACHE=~/cpm-cache

# Build
cmake --build build

# Test
cd build && ctest && cd ..

# Generate coverage trace file
lcov --gcov-tool=gcov-12 --capture --directory $(pwd)'/build' --output-file coverage.info
lcov --gcov-tool=gcov-12 --extract coverage.info $(pwd)'/include/fitoria/*' --output-file coverage.info

# Generate coverage report in html
genhtml -o coverage_report --legend --title "fitoria" --prefix=./ coverage.info
