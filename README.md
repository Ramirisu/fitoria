# Fitoria

[![build](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml/badge.svg)](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml)
[![codecov](https://codecov.io/gh/Ramirisu/fitoria/branch/main/graph/badge.svg?token=YDZ6KGEV0A)](https://codecov.io/gh/Ramirisu/fitoria)
![std](https://img.shields.io/badge/std-20-blue.svg)
![license](https://img.shields.io/badge/license-BSL--1.0-blue)

**fitoria** is an HTTP web framework built on top of C++20 coroutine.

## Quick Start

[More example](https://github.com/Ramirisu/fitoria/tree/main/example)

```cpp

#include <fitoria/fitoria.hpp>

#include <fitoria_certificate.h>

using namespace fitoria;

int main()
{
  log::global_logger() = stdout_logger();
  log::global_logger()->set_log_level(log::level::debug);

  auto server = http_server(http_server_config().route(
      router(http::verb::get, "/api/v1/{owner}/{repo}",
             [](http_request& req) -> net::awaitable<http_response> {
               log::debug("route: {}", req.route().path());
               log::debug("owner: {}, repo: {}", req.route().get("owner"),
                          req.route().get("repo"));

               co_return http_response(http::status::ok)
                   .set_header(http::field::content_type, "text/plain")
                   .set_body("quick start");
             })));
  server
      // Start to listen to port 8080
      .bind("127.0.0.1", 8080)
#if defined(FITORIA_HAS_OPENSSL)
      // Start to listen to port 8443 with SSL enabled
      .bind_ssl("127.0.0.1", 8443,
                cert::get_server_ssl_ctx(net::ssl::context::method::tls_server))
#endif
      // Notify workers to start the IO loop
      // Notice that `run()` will not block current thread
      .run();

  // Register signals to terminate the server
  net::signal_set signal(server.get_execution_context(), SIGINT, SIGTERM);
  signal.async_wait([&](auto, auto) { server.stop(); });

  // Block current thread, current thread will process the IO loop
  server.wait();

  return 0;
}

```

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
