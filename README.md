# Fitoria

[![build](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml/badge.svg)](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml)
[![codecov](https://codecov.io/gh/Ramirisu/fitoria/branch/main/graph/badge.svg?token=YDZ6KGEV0A)](https://codecov.io/gh/Ramirisu/fitoria)
![std](https://img.shields.io/badge/std-20-blue.svg)
![license](https://img.shields.io/badge/license-BSL--1.0-blue)

**fitoria** is an HTTP web framework built on top of C++20 coroutine.

## Examples

### Quick Start
#### HTTP Server

[Quick Start Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/quick_start.cpp)

```cpp

#include <fitoria/fitoria.hpp>

int main()
{
  log::global_logger() = log::stdout_logger();
  log::global_logger()->set_log_level(log::level::debug);

  auto server
      = http_server::builder()
            .route(router(
                http::verb::get, "/api/v1/{owner}/{repo}",
                [](http_request& req) -> net::awaitable<http_response> {
                  log::debug("route: {}", req.route().path());
                  log::debug("owner: {}, repo: {}", req.route().get("owner"),
                             req.route().get("repo"));

                  co_return http_response(http::status::ok)
                      .set_header(http::field::content_type, "text/plain")
                      .set_body("quick start");
                }))
            .build();
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

#### HTTP Client

TODO:


### HTTP Server

#### Methods

Register `GET`, `POST`, `PUT`, `PATCH`, `DELETE` by using `http::verb::*`.

```cpp

int main() {
  auto server = http_server::builder()
      .route(router(http::verb::get, "/get", [](http_request& req)
        -> net::awaitable<http_response> {
        co_return http_response(http::status::ok);
      }))
      .route(router(http::verb::post, "/post", [](http_request& req)
        -> net::awaitable<http_response> {
        co_return http_response(http::status::ok);
      }))
      .route(router(http::verb::put, "/put", [](http_request& req)
        -> net::awaitable<http_response> {
        co_return http_response(http::status::ok);
      }))
      .route(router(http::verb::path, "/path", [](http_request& req)
        -> net::awaitable<http_response> {
        co_return http_response(http::status::ok);
      }))
      .route(router(http::verb::delete_, "/delete", [](http_request& req)
        -> net::awaitable<http_response> {
        co_return http_response(http::status::ok);
      }))
    .build();
}

```

#### Route Parameters

[Route Parameters Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/route_parameter.cpp)

```cpp

int main()
{
  auto server
      = http_server::builder()
            .route(router(
                http::verb::get, "/api/v1/users/{user}",
                [](const http_request& req) -> net::awaitable<http_response> {
                  auto user = req.route().get("user");
                  if (!user) {
                    co_return http_response(http::status::bad_request);
                  }

                  co_return http_response(http::status::ok)
                      .set_header(http::field::content_type, "text/plain")
                      .set_body(fmt::format("user: {}", user.value()));
                }))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();
}

```

#### Query String

[Query String Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/query_string.cpp)

```cpp

int main()
{
  auto server
      = http_server::builder()
            .route(router(
                http::verb::get, "/api/v1/users",
                [](const http_request& req) -> net::awaitable<http_response> {
                  auto user = req.query().get("user");
                  if (!user) {
                    co_return http_response(http::status::bad_request);
                  }

                  co_return http_response(http::status::ok)
                      .set_header(http::field::content_type, "text/plain")
                      .set_body(fmt::format("user: {}", user.value()));
                }))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();
}

```

#### Urlencoded Post Form

[Form Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/form.cpp)

```cpp

int main()
{
  auto server
      = http_server::builder()
            .route(router(
                http::verb::post, "/api/v1/login",
                [](const http_request& req) -> net::awaitable<http_response> {
                  auto user = req.body_as_form();
                  if (!user || user->get("name") != "ramirisu"
                      || user->get("password") != "123456") {
                    co_return http_response(http::status::bad_request);
                  }

                  co_return http_response(http::status::ok);
                }))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();
}

```

#### Multipart

TODO:


#### Json

fitoria integrates `boost::json` as the built-in json serializer/deserializer.

[Json Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/json.cpp)

```cpp

struct user_t {
  std::string name;
  std::string password;

  friend bool operator==(const user_t&, const user_t&) = default;
};

user_t tag_invoke(const json::value_to_tag<user_t>&, const json::value& jv)
{
  return user_t {
    .name = std::string(jv.at("name").as_string()),
    .password = std::string(jv.at("password").as_string()),
  };
}

void tag_invoke(const json::value_from_tag&,
                json::value& jv,
                const user_t& user)
{
  jv = {
    { "name", user.name },
    { "password", user.password },
  };
}

int main()
{
  auto server
      = http_server::builder()
            .route(router(
                http::verb::post, "/api/v1/login",
                [](const http_request& req) -> net::awaitable<http_response> {
                  auto user = req.body_as_json<user_t>();
                  if (!user) {
                    co_return http_response(http::status::bad_request)
                        .set_json({ { "msg", user.error().message() } });
                  }
                  if (user->name != "ramirisu" || user->password != "123456") {
                    co_return http_response(http::status::unauthorized)
                        .set_json({ { "msg",
                                      "user name or password is incorrect" } });
                  }
                  co_return http_response(http::status::ok)
                      .set_json({ { "msg", "login succeeded" } });
                }))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();
}

```

#### Scope

Configure nested routers by using `scope`.

[Router Group Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/scope.cpp)


#### Middleware

`scope` supports `use` to configure middlewares for its `router`s.

[Middleware Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/middleware.cpp)

```cpp

int main()
{
  log::global_logger() = log::stdout_logger();
  log::global_logger()->set_log_level(log::level::debug);

  auto server
      = http_server::builder()
            .route(
                // Add a router group
                scope("/api/v1")
                    // Register built-in logger middleware
                    .use(middleware::logger())
                    // Register built-in exception_handler middleware
                    .use(middleware::exception_handler())
                    // Register a custom middleware for this group
                    .use([](http_context& c) -> net::awaitable<http_response> {
                      log::debug("before handler");
                      auto res = co_await c.next();
                      log::debug("after handler");
                      co_return res;
                    })
                    // Register a route
                    // The route is associated with the middleware defined above
                    .route(router(
                        http::verb::get, "/users/{user}",
                        [](http_request& req) -> net::awaitable<http_response> {
                          FITORIA_ASSERT(req.method() == http::verb::get);

                          log::debug("user: {}", req.route().get("user"));

                          co_return http_response(http::status::ok)
                              .set_body("abcde");
                        })))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();
}

```

### HTTP Client

TODO:

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
