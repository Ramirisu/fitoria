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

|    Library     | Usage                              |              Namespace              |              required/optional              |
| :------------: | :--------------------------------- | :---------------------------------: | :-----------------------------------------: |
| `boost::asio`  | networking                         |           `fitoria::net`            |                  required                   |
| `boost::beast` | http                               | `fitoria::net`<br />`fitoria::http` |                  required                   |
|  `boost::url`  | internal url parsing               |           `fitoria::urls`           |                  required                   |
| `boost::json`  | json serialization/deserialization |           `fitoria::json`           |                  required                   |
|     `fmt`      | formatting                         |           `fitoria::fmt`            | required (if `std.format` is not available) |
|   `OpenSSL`    | secure networking                  |                none                 |                  optional                   |
|   `doctest`    | unittesting                        |                none                 |                  optional                   |

CMake

| Option                  | Description                              | Value  | Default |
| :---------------------- | :--------------------------------------- | :----: | :-----: |
| FITORIA_BUILD_EXAMPLES  | Build examples                           | ON/OFF |   OFF   |
| FITORIA_BUILD_TESTS     | Build tests                              | ON/OFF |   OFF   |
| FITORIA_DISABLE_OPENSSL | Do not use OpenSSL                       | ON/OFF |   OFF   |
| FITORIA_DISABLE_CPM     | Do not use CPM.cmake to download library | ON/OFF |   OFF   |

* `fitoria` calls `find_package` to search the dependencies first. If not found, will try to use CPM.cmake to download the dependencies.

```sh
git clone https://github.com/Ramirisu/fitoria.git
cd fitoria/
cmake -B build -DFITORIA_BUILD_EXAMPLES=ON -DFITORIA_BUILD_TESTS=ON
cmake --build build
cd build && ctest && cd ..
```

## License

This project is distributed under the [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt).
