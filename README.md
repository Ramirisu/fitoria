# Fitoria

[![build](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml/badge.svg)](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml)
[![codecov](https://codecov.io/gh/Ramirisu/fitoria/branch/main/graph/badge.svg?token=YDZ6KGEV0A)](https://codecov.io/gh/Ramirisu/fitoria)
![std](https://img.shields.io/badge/std-20-blue.svg)
![platform](https://img.shields.io/badge/platform-windows%2Flinux%2Fmacos-blue)
![license](https://img.shields.io/badge/license-BSL--1.0-blue)

**fitoria** is a modern C++20, cross-platform web framework.

The library is ***experimental*** and still under development, not recommended for production use.

## Table of Contents

- [Fitoria](#fitoria)
  - [Table of Contents](#table-of-contents)
  - [Documentation](#documentation)
  - [Building](#building)
  - [License](#license)

## Documentation

TODO:

## Building

Platform

- Linux
- Windows
- MacOS

Compiler (C++20)

- GCC 13
- MSVC 17 2022
- Clang 15

Dependencies

|    Library     | Minimum Version |          |
| :------------: | :-------------: | :------: |
| `boost::asio`  |     `1.84`      | required |
| `boost::beast` |     `1.84`      | required |
| `boost::json`  |     `1.84`      | required |
|  `boost::url`  |     `1.84`      | required |
|  `boost::pfr`  |     `1.84`      | required |
|     `fmt`      |     `10.x`      | required |
|     `zlib`     |                 | optional |
|    `brotli`    |                 | optional |
|   `OpenSSL`    |                 | optional |
|   `doctest`    |                 | optional |

CMake

| Option                    | Description                              | Value  | Default |
| :------------------------ | :--------------------------------------- | :----: | :-----: |
| FITORIA_BUILD_EXAMPLES    | Build examples                           | ON/OFF |   OFF   |
| FITORIA_BUILD_TESTS       | Build tests                              | ON/OFF |   OFF   |
| FITORIA_DISABLE_OPENSSL   | Do not enable OpenSSL dependent features | ON/OFF |   OFF   |
| FITORIA_DISABLE_ZLIB      | Do not enable ZLIB dependent features    | ON/OFF |   OFF   |
| FITORIA_DISABLE_BROTLI    | Do not enable Brotli dependent features  | ON/OFF |   OFF   |
| FITORIA_HAS_LIBURING      | Do not enable liburing                   | ON/OFF |   OFF   |
| FITORIA_ENABLE_CODECOV    | Enable code coverage build               | ON/OFF |   OFF   |
| FITORIA_ENABLE_CLANG_TIDY | Enable clang-tidy check                  | ON/OFF |   OFF   |

```sh

git clone https://github.com/Ramirisu/fitoria.git
cd fitoria/
cmake -B build -DFITORIA_BUILD_EXAMPLES=ON -DFITORIA_BUILD_TESTS=ON
cmake --build build
cd build && ctest && cd ..

```

## License

This project is distributed under the [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt).
