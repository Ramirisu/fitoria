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
  - [Features](#features)
  - [Getting Started](#getting-started)
  - [Building](#building)
  - [License](#license)

## Features

- Ease to use: APIs are designed based on C++20 coroutine and highly integrated with `optional` and `expected` to provide better error handling mechanisms
- Cross-platform, write once and run across Windows, Linux and MacOS
- Support HTTP/1.1
- Support SSL up to TLS 1.3
- Support Unix Domain Socket
- Support WebSocket

More details can be found in the [documentation](https://ramirisu.github.io/fitoria/)

## Getting Started

The following example demonstrates how to create a simple ``http_server`` and attach handlers to it. ([Getting Started Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/getting_started.cpp))

```cpp

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

auto hello_world() -> awaitable<response>
{
  co_return response::ok()
      .set_header(http::field::content_type,
                  http::fields::content_type::plaintext())
      .set_body("Hello World!");
}

auto echo(std::string body) -> awaitable<response>
{
  co_return response::ok()
      .set_header(http::field::content_type,
                  http::fields::content_type::plaintext())
      .set_body(body);
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">(hello_world))
                    .serve(route::post<"/echo">(echo))
                    .build();

  server.bind("127.0.0.1", 8080);

  ioc.run();
}

```

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
| `boost::asio`  |    `1.84.0`     | required |
| `boost::beast` |    `1.84.0`     | required |
| `boost::json`  |    `1.84.0`     | required |
|  `boost::url`  |    `1.84.0`     | required |
|  `boost::pfr`  |    `1.84.0`     | required |
|     `fmt`      |    `10.0.0`     | required |
|     `zlib`     |                 | optional |
|    `brotli`    |                 | optional |
|   `OpenSSL`    |                 | optional |
|   `doctest`    |                 | optional |

CMake

| Option                           | Description                              | Value  | Default |
| :------------------------------- | :--------------------------------------- | :----: | :-----: |
| FITORIA_BUILD_EXAMPLES           | Build examples                           | ON/OFF |   OFF   |
| FITORIA_BUILD_TESTS              | Build tests                              | ON/OFF |   OFF   |
| FITORIA_DISABLE_OPENSSL          | Do not enable OpenSSL dependent features | ON/OFF |   OFF   |
| FITORIA_DISABLE_ZLIB             | Do not enable ZLIB dependent features    | ON/OFF |   OFF   |
| FITORIA_DISABLE_BROTLI           | Do not enable Brotli dependent features  | ON/OFF |   OFF   |
| FITORIA_DISABLE_LIBURING         | Do not enable liburing                   | ON/OFF |   OFF   |
| FITORIA_ENABLE_CODECOV           | Enable code coverage build               | ON/OFF |   OFF   |
| FITORIA_ENABLE_CLANG_TIDY        | Enable clang-tidy check                  | ON/OFF |   OFF   |
| FITORIA_ENABLE_ADDRESS_SANITIZER | Compile with `-fsanitize=address`        | ON/OFF |   OFF   |

```sh

git clone https://github.com/Ramirisu/fitoria.git
cd fitoria/
cmake -B build -DFITORIA_BUILD_EXAMPLES=ON -DFITORIA_BUILD_TESTS=ON
cmake --build build
cd build && ctest && cd ..

```

## License

This project is distributed under the [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt).
