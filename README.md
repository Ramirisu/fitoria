# Fitoria

[![build](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml/badge.svg)](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml)
[![codecov](https://codecov.io/gh/Ramirisu/fitoria/branch/main/graph/badge.svg?token=YDZ6KGEV0A)](https://codecov.io/gh/Ramirisu/fitoria)
![std](https://img.shields.io/badge/std-20-blue.svg)
![platform](https://img.shields.io/badge/platform-windows%2Flinux%2Fmacos-blue)
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
      - [Method](#method)
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
            .route(route::GET(
                "/api/v1/{owner}/{repo}",
                [](http_request& req) -> net::awaitable<http_response> {
                  log::debug("route: {}", req.params().path());
                  log::debug("owner: {}, repo: {}", req.params().get("owner"),
                             req.params().get("repo"));

                  co_return http_response(http::status::ok)
                      .set_field(http::field::content_type,
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

#### Method

Register `GET`, `POST`, `PUT`, `PATCH`, `DELETE`, `HEAD` and `OPTIONS`.

[Method Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/method.cpp)

```cpp

int main()
{
  auto server = http_server::builder()
                    // Single route by using `route`
                    .route(route::handle(http::verb::get, "/", get))
                    .route(route::GET("/get", get))
                    .route(route::POST("/post", post))
                    .route(route::PUT("/put", put))
                    .route(route::PATCH("/patch", patch))
                    .route(route::DELETE_("/delete", delete_))
                    .route(route::HEAD("/head", head))
                    .route(route::OPTIONS("/options", options))
                    // Grouping routes by using `scope`
                    .route(scope("/api/v1")
                               .handle(http::verb::get, "/", get)
                               .GET("/get", get)
                               .POST("/post", post)
                               .PUT("/put", put)
                               .PATCH("/patch", patch)
                               .DELETE_("/delete", delete_)
                               .HEAD("/head", head)
                               .OPTIONS("/options", options))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Route Parameters

Use `http_request::route_params()` to access the route parameters.

[Route Parameters Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/route_parameter.cpp)

```cpp

namespace api::v1::users::get_user {
auto api(const http_request& req) -> net::awaitable<http_response>
{
  auto user = req.params().get("user");
  if (!user) {
    co_return http_response(http::status::bad_request);
  }

  co_return http_response(http::status::ok)
      .set_field(http::field::content_type,
                 http::fields::content_type::plaintext())
      .set_body(fmt::format("user: {}", user.value()));
}
}

int main()
{
  auto server = http_server::builder()
                    .route(route::GET("/api/v1/users/{user}",
                                      api::v1::users::get_user::api))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Query String Parameters

Use `http_request::query()` to access the query string parameters.

[Query String Parameters Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/query_string.cpp)

```cpp

namespace api::v1::users::get_user {
auto api(const http_request& req) -> net::awaitable<http_response>
{
  auto user = req.query().get("user");
  if (!user) {
    co_return http_response(http::status::bad_request);
  }

  co_return http_response(http::status::ok)
      .set_field(http::field::content_type,
                 http::fields::content_type::plaintext())
      .set_body(fmt::format("user: {}", user.value()));
}
}

int main()
{
  auto server
      = http_server::builder()
            .route(route::GET("/api/v1/users", api::v1::users::get_user::api))
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

namespace api::v1::login {
auto api(const http_request& req) -> net::awaitable<http_response>
{
  if (req.fields().get(http::field::content_type)
      != http::fields::content_type::form_urlencoded()) {
    co_return http_response(http::status::bad_request);
  }
  auto user = as_form(req.body());
  if (!user || user->get("name") != "ramirisu"
      || user->get("password") != "123456") {
    co_return http_response(http::status::unauthorized);
  }

  co_return http_response(http::status::ok);
}
}

int main()
{
  auto server = http_server::builder()
                    .route(route::POST("/api/v1/login", api::v1::login::api))
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

namespace api::v1::login {
struct user_t {
  std::string name;
  std::string password;
};

json::result_for<user_t, json::value>::type
tag_invoke(const json::try_value_to_tag<user_t>&, const json::value& jv)
{
  user_t user;

  if (!jv.is_object()) {
    return make_error_code(json::error::incomplete);
  }

  const auto& obj = jv.get_object();

  auto* name = obj.if_contains("name");
  auto* password = obj.if_contains("password");
  if (name && password && name->is_string() && password->is_string()) {
    return user_t { .name = std::string(name->get_string()),
                    .password = std::string(password->get_string()) };
  }

  return make_error_code(json::error::incomplete);
}

struct output {
  std::string msg;
};

void tag_invoke(const json::value_from_tag&, json::value& jv, const output& out)
{
  jv = { { "msg", out.msg } };
}

auto api(const http_request& req) -> net::awaitable<http_response>
{
  if (auto ct = req.fields().get(http::field::content_type);
      ct != http::fields::content_type::json()) {
    co_return http_response(http::status::bad_request)
        .set_json(output { .msg = "unexpected Content-Type" });
  }
  auto user = as_json<user_t>(req.body());
  if (!user) {
    co_return http_response(http::status::bad_request)
        .set_json(output { .msg = user.error().message() });
  }
  if (user->name != "ramirisu" || user->password != "123456") {
    co_return http_response(http::status::unauthorized)
        .set_json(output { .msg = "user name or password is incorrect" });
  }
  co_return http_response(http::status::ok)
      .set_json(output { .msg = "login succeeded" });
}
}

int main()
{
  auto server = http_server::builder()
                    .route(route::POST("/api/v1/login", api::v1::login::api))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Scope

Configure nested `route`s by using `scope`.

[Scope Example](https://github.com/Ramirisu/fitoria/blob/main/example/web/scope.cpp)

```cpp

void configure_application(http_server::builder& builder)
{
  builder.route(
      // Global scope
      scope("")
          // Register a global middleware for all handlers
          .use(middleware::logger())
          // Create a sub-scope "/api/v1" under global scope
          .sub_scope(
              scope("/api/v1")
                  // Register a middleware for this scope
                  .use(middleware::gzip())
                  // Register a route for this scope
                  .GET("/users/{user}",
                       [](http_request& req) -> net::awaitable<http_response> {
                         log::debug("route: {}", req.params().path());

                         co_return http_response(http::status::ok);
                       }))
          // Create a sub-scope "/api/v2" under global scope
          .sub_scope(
              scope("/api/v2")
                  // Register a middleware for this scope
                  .use(middleware::deflate())
                  // Register a route for this scope
                  .GET("/users/{user}",
                       [](http_request& req) -> net::awaitable<http_response> {
                         log::debug("params: {}", req.params().path());

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

fitoria provides following build-in middlewares:

* `middleware::logger`
* `middleware::exception_handler`
* `middleware::deflate`
* `middleware::gzip`

```cpp

template <typename Next>
class my_log_service {
  Next next_;

public:
  my_log_service(Next next)
      : next_(std::move(next))
  {
  }

  auto operator()(http_context& ctx) const -> net::awaitable<http_response>
  {
    log::debug("before handler");

    auto res = co_await next_(ctx);

    log::debug("after handler");

    co_return res;
  }
};

class my_log {
public:
  template <typename Next>
  auto create(Next next) const
  {
    return my_log_service(std::move(next));
  }
};

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
#if !FITORIA_NO_EXCEPTIONS
                    // Register built-in exception_handler middleware
                    .use(middleware::exception_handler())
#endif
                    // Register built-in deflate middleware
                    .use(middleware::deflate())
#if defined(FITORIA_HAS_ZLIB)
                    // Register built-in gzip middleware
                    .use(middleware::gzip())
#endif
                    // Register a custom middleware for this group
                    .use(my_log())
                    // Register a route
                    // The route is associated with the middleware defined above
                    .GET(
                        "/users/{user}",
                        [](http_request& req) -> net::awaitable<http_response> {
                          log::debug("user: {}", req.params().get("user"));

                          co_return http_response(http::status::ok)
                              .set_body(req.params().get("user").value_or(
                                  "{{unknown}}"));
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
            .route(route::POST(
                "/api/v1/login",
                [](http_request& req) -> net::awaitable<http_response> {
                  if (req.fields().get(http::field::content_type)
                      != http::fields::content_type::form_urlencoded()) {
                    co_return http_response(http::status::bad_request);
                  }
                  auto user = as_form(req.body());
                  if (!user || user->get("name") != "ramirisu"
                      || user->get("password") != "123456") {
                    co_return http_response(http::status::unauthorized);
                  }
                  co_return http_response(http::status::ok)
                      .set_field(http::field::content_type,
                                 http::fields::content_type::plaintext())
                      .set_body(fmt::format("{}, login succeeded",
                                            user->get("name")));
                }))
            .build();

  {
    auto res = server.serve_http_request(
        "/api/v1/login",
        http_request(http::verb::post)
            .set_field(http::field::content_type,
                       http::fields::content_type::plaintext())
            .set_body("name=ramirisu&password=123456"));
    FITORIA_ASSERT(res.status_code() == http::status::bad_request);
  }
  {
    auto res = server.serve_http_request(
        "/api/v1/login",
        http_request(http::verb::post)
            .set_field(http::field::content_type,
                       http::fields::content_type::form_urlencoded())
            .set_body("name=unknown&password=123"));
    FITORIA_ASSERT(res.status_code() == http::status::unauthorized);
  }
  {
    auto res = server.serve_http_request(
        "/api/v1/login",
        http_request(http::verb::post)
            .set_field(http::field::content_type,
                       http::fields::content_type::form_urlencoded())
            .set_body("name=ramirisu&password=123"));
    FITORIA_ASSERT(res.status_code() == http::status::unauthorized);
  }
  {
    auto res = server.serve_http_request(
        "/api/v1/login",
        http_request(http::verb::post)
            .set_field(http::field::content_type,
                       http::fields::content_type::form_urlencoded())
            .set_body("name=ramirisu&password=123456"));
    FITORIA_ASSERT(res.status_code() == http::status::ok);
    FITORIA_ASSERT(res.fields().get(http::field::content_type)
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

Compiler (C++20 coroutine/concepts)

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
