# Fitoria

[![build](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml/badge.svg)](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml)
[![codecov](https://codecov.io/gh/Ramirisu/fitoria/branch/main/graph/badge.svg?token=YDZ6KGEV0A)](https://codecov.io/gh/Ramirisu/fitoria)
![std](https://img.shields.io/badge/std-20-blue.svg)
![license](https://img.shields.io/badge/license-BSL--1.0-blue)

**fitoria** is an HTTP web framework built on top of C++20 coroutine.

## Table of Contents

- [Fitoria](#fitoria)
  - [Table of Contents](#table-of-contents)
  - [Examples](#examples)
    - [Quick Start](#quick-start)
      - [HTTP Server](#http-server)
      - [HTTP Client](#http-client)
    - [HTTP Server](#http-server-1)
      - [Methods](#methods)
      - [Route Parameters](#route-parameters)
      - [Query String Parameters](#query-string-parameters)
      - [Urlencoded Post Form](#urlencoded-post-form)
      - [Multipart](#multipart)
      - [JSON](#json)
      - [Scope](#scope)
      - [Middleware](#middleware)
      - [Graceful Shutdown](#graceful-shutdown)
      - [Unit Testing](#unit-testing)
    - [HTTP Client](#http-client-1)
  - [Building](#building)
  - [License](#license)

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
            .route(
                route(http::verb::get, "/api/v1/{owner}/{repo}",
                      [](http_request& req) -> net::awaitable<http_response> {
                        log::debug("route: {}", req.route_params().path());
                        log::debug("owner: {}, repo: {}",
                                   req.route_params().get("owner"),
                                   req.route_params().get("repo"));

                        co_return http_response(http::status::ok)
                            .set_header(http::field::content_type,
                                        http::fields::content_type::plaintext())
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
      // Start the server, `run()` will block current thread.
      .run();
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
      .route(route(http::verb::get, "/get", [](http_request& req)
        -> net::awaitable<http_response> {
        co_return http_response(http::status::ok);
      }))
      .route(route(http::verb::post, "/post", [](http_request& req)
        -> net::awaitable<http_response> {
        co_return http_response(http::status::ok);
      }))
      .route(route(http::verb::put, "/put", [](http_request& req)
        -> net::awaitable<http_response> {
        co_return http_response(http::status::ok);
      }))
      .route(route(http::verb::path, "/path", [](http_request& req)
        -> net::awaitable<http_response> {
        co_return http_response(http::status::ok);
      }))
      .route(route(http::verb::delete_, "/delete", [](http_request& req)
        -> net::awaitable<http_response> {
        co_return http_response(http::status::ok);
      }))
    .build();
}

```

#### Route Parameters

Use `http_request::route()` to access the route parameters.

[Route Parameters Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/route_parameter.cpp)

```cpp

int main()
{
  auto server
      = http_server::builder()
            .route(route(
                http::verb::get, "/api/v1/users/{user}",
                [](const http_request& req) -> net::awaitable<http_response> {
                  auto user = req.route_params().get("user");
                  if (!user) {
                    co_return http_response(http::status::bad_request);
                  }

                  co_return http_response(http::status::ok)
                      .set_header(http::field::content_type,
                                  http::fields::content_type::plaintext())
                      .set_body(fmt::format("user: {}", user.value()));
                }))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Query String Parameters

Use `http_request::query()` to access the qeury string parameters.

[Query String Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/query_string.cpp)

```cpp

int main()
{
  auto server
      = http_server::builder()
            .route(route(
                http::verb::get, "/api/v1/users",
                [](const http_request& req) -> net::awaitable<http_response> {
                  auto user = req.query().get("user");
                  if (!user) {
                    co_return http_response(http::status::bad_request);
                  }

                  co_return http_response(http::status::ok)
                      .set_header(http::field::content_type,
                                  http::fields::content_type::plaintext())
                      .set_body(fmt::format("user: {}", user.value()));
                }))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Urlencoded Post Form

Use `as_form()` to parse the url-encoded form body.

[Form Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/form.cpp)

```cpp

int main()
{
  auto server
      = http_server::builder()
            .route(route(
                http::verb::post, "/api/v1/login",
                [](const http_request& req) -> net::awaitable<http_response> {
                  if (req.headers().get(http::field::content_type)
                      != http::fields::content_type::form_urlencoded()) {
                    co_return http_response(http::status::bad_request);
                  }
                  auto user = as_form(req.body());
                  if (!user || user->get("name") != "ramirisu"
                      || user->get("password") != "123456") {
                    co_return http_response(http::status::unauthorized);
                  }

                  co_return http_response(http::status::ok);
                }))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Multipart

TODO:


#### JSON

fitoria integrates `boost::json` as the built-in json serializer/deserializer. Use `as_json` to parse the body.

[JSON Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/json.cpp)

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
            .route(route(
                http::verb::post, "/api/v1/login",
                [](const http_request& req) -> net::awaitable<http_response> {
                  if (auto ct = req.headers().get(http::field::content_type);
                      ct != http::fields::content_type::json()) {
                    co_return http_response(http::status::bad_request)
                        .set_json({ { "msg",
                                      fmt::format("expected Content-Type: "
                                                  "application/json, got {}",
                                                  ct) } });
                  }
                  auto user = as_json<user_t>(req.body());
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
}

```

#### Scope

Configure nested routers by using `scope`.

[Scope Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/scope.cpp)

```cpp

namespace my_middleware {

auto log(http_context& c) -> net::awaitable<http_response>
{
  log::debug("log middleware (in)");
  auto res = co_await c.next();
  log::debug("log middleware (out)");
  co_return res;
}

namespace v1 {
  auto auth(http_context& c) -> net::awaitable<http_response>
  {
    log::debug("v1 auth middleware (in)");
    auto res = co_await c.next();
    log::debug("v1 auth middleware (out)");
    co_return res;
  }
}

namespace v2 {
  auto auth(http_context& c) -> net::awaitable<http_response>
  {
    log::debug("v2 auth middleware (in)");
    auto res = co_await c.next();
    log::debug("v2 auth middleware (out)");
    co_return res;
  }
}
}

void configure_application(http_server::builder& builder)
{
  builder.route(
      // Global scope
      scope("")
          // Register a global middleware for all handlers
          .use(my_middleware::log)
          // Create a sub-scope "/api/v1" under global scope
          .sub_scope(scope("/api/v1")
                         // Register a middleware for this scope
                         .use(my_middleware::v1::auth)
                         // Register a route for this scope
                         .route(http::verb::get, "/users/{user}",
                                []([[maybe_unused]] http_request& req)
                                    -> net::awaitable<http_response> {
                                  log::debug("route: {}",
                                             req.route_params().path());

                                  co_return http_response(http::status::ok);
                                }))
          // Create a sub-scope "/api/v2" under global scope
          .sub_scope(scope("/api/v2")
                         // Register a middleware for this scope
                         .use(my_middleware::v2::auth)
                         // Register a route for this scope
                         .route(http::verb::get, "/users/{user}",
                                []([[maybe_unused]] http_request& req)
                                    -> net::awaitable<http_response> {
                                  log::debug("route_params: {}",
                                             req.route_params().path());

                                  co_return http_response(http::status::ok);
                                })));
}

int main()
{
  log::global_logger() = log::stdout_logger();
  log::global_logger()->set_log_level(log::level::debug);

  auto server = http_server::builder()
                    // Use a configure function to setup server configuration
                    .configure(configure_application)
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Middleware

`scope` supports `use` to configure middlewares for its `router`s.

([Middleware Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/middleware.cpp))

fitoria provides following build-in middlewares

* `middleware::logger`
* `middleware::exception_handler`
* `middleware::deflate`
* `middleware::gzip`

```cpp

int main()
{
  log::global_logger() = log::stdout_logger();
  log::global_logger()->set_log_level(log::level::debug);

  auto server
      = http_server::builder()
            .route(
                // Add a scope
                scope("/api/v1")
                    // Register built-in logger middleware
                    .use(middleware::logger())
                    // Register built-in exception_handler middleware
                    .use(middleware::exception_handler())
                    // Register built-in deflate middleware
                    .use(middleware::deflate())
#if defined(FITORIA_HAS_ZLIB)
                    // Register built-in gzip middleware
                    .use(middleware::gzip())
#endif
                    // Register a custom middleware for this group
                    .use([](http_context& c) -> net::awaitable<http_response> {
                      log::debug("before handler");
                      auto res = co_await c.next();
                      log::debug("after handler");
                      co_return res;
                    })
                    // Register a route
                    // The route is associated with the middleware defined above
                    .route(
                        http::verb::get, "/users/{user}",
                        [](http_request& req) -> net::awaitable<http_response> {
                          FITORIA_ASSERT(req.method() == http::verb::get);

                          log::debug("user: {}",
                                     req.route_params().get("user"));

                          co_return http_response(http::status::ok)
                              .set_body("abcde");
                        }))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```
#### Graceful Shutdown

Use `net::signal_set` to handle signals to shutdown the server gracefully.

([Graceful Shutdown Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/graceful_shutdown.cpp))

```cpp

int main()
{
  auto server = http_server::builder().build();
  server.bind("127.0.0.1", 8080);

  // Create the execution context and spawn coroutines on it
  net::io_context ioc;
  net::co_spawn(ioc, server.async_run(), net::detached);

  // Register signals to stop the execution context
  net::signal_set signal(ioc, SIGINT, SIGTERM);
  signal.async_wait([&](auto, auto) { ioc.stop(); });

  // Run the execution context with thread pool
  const std::uint32_t num_threads = std::thread::hardware_concurrency();
  net::thread_pool tp(num_threads);
  for (auto i = std::uint32_t(1); i < num_threads; ++i) {
    net::post(tp, [&]() { ioc.run(); });
  }
  ioc.run();
}

```

#### Unit Testing

`http::serve_http_request()` can consume the mock `http_request` directly without creating TCP connections. 

([Unit Testing Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/unittesting.cpp))

```cpp

int main()
{
  auto server
      = http_server::builder()
            .route(
                route(http::verb::post, "/api/v1/login",
                      [](http_request& req) -> net::awaitable<http_response> {
                        if (req.headers().get(http::field::content_type)
                            != http::fields::content_type::form_urlencoded()) {
                          co_return http_response(http::status::bad_request);
                        }
                        auto user = as_form(req.body());
                        if (!user || user->get("name") != "ramirisu"
                            || user->get("password") != "123456") {
                          co_return http_response(http::status::unauthorized);
                        }
                        co_return http_response(http::status::ok)
                            .set_header(http::field::content_type,
                                        http::fields::content_type::plaintext())
                            .set_body(fmt::format("{}, login succeeded",
                                                  user->get("name")));
                      }))
            .build();

  {
    auto res = server.serve_http_request(
        "/api/v1/login",
        http_request()
            .set_method(http::verb::post)
            .set_header(http::field::content_type,
                        http::fields::content_type::plaintext())
            .set_body("name=ramirisu&password=123456"));
    FITORIA_ASSERT(res.status_code() == http::status::bad_request);
  }
  {
    auto res = server.serve_http_request(
        "/api/v1/login",
        http_request()
            .set_method(http::verb::post)
            .set_header(http::field::content_type,
                        http::fields::content_type::form_urlencoded())
            .set_body("name=unknown&password=123"));
    FITORIA_ASSERT(res.status_code() == http::status::unauthorized);
  }
  {
    auto res = server.serve_http_request(
        "/api/v1/login",
        http_request()
            .set_method(http::verb::post)
            .set_header(http::field::content_type,
                        http::fields::content_type::form_urlencoded())
            .set_body("name=ramirisu&password=123"));
    FITORIA_ASSERT(res.status_code() == http::status::unauthorized);
  }
  {
    auto res = server.serve_http_request(
        "/api/v1/login",
        http_request()
            .set_method(http::verb::post)
            .set_header(http::field::content_type,
                        http::fields::content_type::form_urlencoded())
            .set_body("name=ramirisu&password=123456"));
    FITORIA_ASSERT(res.status_code() == http::status::ok);
    FITORIA_ASSERT(res.headers().get(http::field::content_type)
                   == http::fields::content_type::plaintext());
    FITORIA_ASSERT(res.body() == "ramirisu, login succeeded");
  }
}

```

### HTTP Client

TODO:

## Building

Platform

- Linux
- Windows
- MacOS

Compiler

- GCC 12
- MSVC 17 2022
- Clang 15

Dependencies

|    Library     | Usage                              |              Namespace              | required/optional |
| :------------: | :--------------------------------- | :---------------------------------: | :---------------: |
| `boost::asio`  | Networking                         |           `fitoria::net`            |     required      |
| `boost::beast` | HTTP                               | `fitoria::net`<br />`fitoria::http` |     required      |
|  `boost::url`  | Internal url parsing               |           `fitoria::urls`           |     required      |
| `boost::json`  | JSON serialization/deserialization |           `fitoria::json`           |     required      |
|     `zlib`     | Built-in middleware gzip           |                none                 |     optional      |
|     `fmt`      | Formatting                         |           `fitoria::fmt`            |     optional      |
|   `OpenSSL`    | Secure networking                  |                none                 |     optional      |
|   `doctest`    | Unit testing                       |                none                 |     optional      |

CMake

| Option                  | Description                | Value  | Default |
| :---------------------- | :------------------------- | :----: | :-----: |
| FITORIA_BUILD_EXAMPLES  | Build examples             | ON/OFF |   OFF   |
| FITORIA_BUILD_TESTS     | Build tests                | ON/OFF |   OFF   |
| FITORIA_DISABLE_OPENSSL | Do not use OpenSSL         | ON/OFF |   OFF   |
| FITORIA_DISABLE_ZLIB    | Do not use ZLIB            | ON/OFF |   OFF   |
| FITORIA_ENABLE_CODECOV  | Enable code coverage build | ON/OFF |   OFF   |

```sh
git clone https://github.com/Ramirisu/fitoria.git
cd fitoria/
cmake -B build -DFITORIA_BUILD_EXAMPLES=ON -DFITORIA_BUILD_TESTS=ON
cmake --build build
cd build && ctest && cd ..
```

## License

This project is distributed under the [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt).
