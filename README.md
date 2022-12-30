# Fitoria

[![build](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml/badge.svg)](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml)
![std](https://img.shields.io/badge/std-20-blue.svg)
![license](https://img.shields.io/badge/license-BSL--1.0-blue)

`fitoria` is an HTTP web framework built on top of C++20 coroutines.

## Building

Compiler Supported

* GCC 12
* MSVC 17 2022

Dependencies

* `boost::asio` for networking (Required)
* `boost::beast` for http/websocket (Required)
* `boost::url` for url parsing (Required)
* `boost::json` for json serialization/deserialization (Required)
* `OpenSSL` for secure networking (Optional)
* `doctest` for unittesting (Optional)

CMake

| Option                  | Description        | Value  | Default |
| :---------------------- | :----------------- | :----: | :-----: |
| FITORIA_BUILD_EXAMPLES  | Build examples     | ON/OFF |   OFF   |
| FITORIA_BUILD_TESTS     | Build tests        | ON/OFF |   OFF   |
| FITORIA_DISABLE_OPENSSL | Do not use OpenSSL | ON/OFF |   OFF   |

```sh
git clone https://github.com/Ramirisu/fitoria.git
cd fitoria/
cmake -B build -DFITORIA_BUILD_EXAMPLES=ON -DFITORIA_BUILD_TESTS=ON -DFITORIA_DISABLE_OPENSSL=OFF
cmake --build build
cd build && ctest && cd ..
```

## License

This project is distributed under the [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt).
