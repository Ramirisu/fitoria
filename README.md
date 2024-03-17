# Fitoria

[![build](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml/badge.svg)](https://github.com/Ramirisu/fitoria/actions/workflows/build_matrix.yml)
[![codecov](https://codecov.io/gh/Ramirisu/fitoria/branch/main/graph/badge.svg?token=YDZ6KGEV0A)](https://codecov.io/gh/Ramirisu/fitoria)
![std](https://img.shields.io/badge/std-20-blue.svg)
![platform](https://img.shields.io/badge/platform-windows%2Flinux%2Fmacos-blue)
![license](https://img.shields.io/badge/license-BSL--1.0-blue)

**fitoria** is an HTTP web framework built on top of C++20 coroutine.

The library is ***experimental*** and still under development, not recommended for production use.

## Table of Contents

- [Fitoria](#fitoria)
  - [Table of Contents](#table-of-contents)
  - [Examples](#examples)
    - [Quick Start](#quick-start)
      - [HTTP Server](#http-server)
      - [HTTP Client](#http-client)
    - [HTTP Server](#http-server-1)
      - [Method](#method)
      - [Route](#route)
      - [Path Parameter](#path-parameter)
      - [Query String Parameters](#query-string-parameters)
      - [Urlencoded Post Form](#urlencoded-post-form)
      - [Shared States](#shared-states)
      - [Extractor](#extractor)
      - [Scope](#scope)
      - [Middleware](#middleware)
      - [Graceful Shutdown](#graceful-shutdown)
      - [WebSockets](#websockets)
      - [Unit Testing](#unit-testing)
    - [HTTP Client](#http-client-1)
  - [Building](#building)
  - [License](#license)

## Examples

### Quick Start

#### HTTP Server

[Code](https://github.com/Ramirisu/fitoria/blob/main/example/web/getting_started.cpp)

```cpp

#include <fitoria/fitoria.hpp>

using namespace fitoria;
using namespace fitoria::web;

int main()
{
  log::global_logger() = log::stdout_logger();
  log::global_logger()->set_log_level(log::level::debug);

  auto server
      = http_server::builder()
            .serve(route::get<"/api/v1/{owner}/{repo}">(
                [](http_request& req) -> net::awaitable<http_response> {
                  log::debug("route: {}", req.params().path());
                  log::debug("owner: {}, repo: {}",
                             req.params().get("owner"),
                             req.params().get("repo"));

                  co_return http_response(http::status::ok)
                      .set_field(http::field::content_type,
                                 http::fields::content_type::plaintext())
                      .set_body("getting started");
                }))
            .build();
  server
      // Start to listen to port 8080
      .bind("127.0.0.1", 8080)
#if defined(FITORIA_HAS_OPENSSL)
      // Start to listen to port 8443 with SSL enabled
      .bind_ssl("127.0.0.1",
                8443,
                cert::get_server_ssl_ctx(net::ssl::context::method::tls_server))
#endif
      // Start the server, `run()` will block current thread.
      .run();
}

```

#### HTTP Client

***TODO:***

### HTTP Server

#### Method

Register methods defined in `http::verb::*` by using `route::handle`, or simply use `route::get`, `route::post`, `route::put`, `route::patch`, `route::delete_`, `route::head` and `route::options` for convenience. `route::any` can register a handler to serve any method. ([Code](https://github.com/Ramirisu/fitoria/blob/main/example/web/method.cpp))

```cpp

int main()
{
  auto server = http_server::builder()
                    // Single route by using `route`
                    .serve(route::handle<"/">(http::verb::get, get_handler))
                    .serve(route::get<"/get">(get_handler))
                    .serve(route::post<"/post">(post_handler))
                    .serve(route::put<"/put">(put_handler))
                    .serve(route::patch<"/patch">(patch_handler))
                    .serve(route::delete_<"/delete">(delete_handler))
                    .serve(route::head<"/head">(head_handler))
                    .serve(route::options<"/options">(options_handler))
                    .serve(route::any<"/any">(any_handler))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Route

Support **static path**, **parameterized path** and **wildcard matching**. Perform **compile-time validation** for path.

```cpp

route::get<"/api/v1/get">(handler) // static
route::get<"/api/v1/get/{param}">(handler) // path parameter, `web::path_info::get("param")`
route::get<"/api/v1/#any_path">(handler) // wildcard matching, `web::path_info::get("any_path")`

route::get<"/api/v1/{">(handler) // error: static_assert failed: 'invalid path for route'
route::get<"/api/v1/}">(handler) // error: static_assert failed: 'invalid path for route'
route::get<"/api/v1/{param}x">(handler) // error: static_assert failed: 'invalid path for route'

```

Route 

|     Type      | Priority |         Example         | Format                                                                                   |                                                                                                                   |
| :-----------: | :------: | :---------------------: | :--------------------------------------------------------------------------------------- | :---------------------------------------------------------------------------------------------------------------- |
|    Static     |    1     | `/api/v1/user/ramirisu` |                                                                                          |                                                                                                                   |
| Parameterized |    2     |  `/api/v1/user/{user}`  | A name parameter enclosed within `{}`.                                                   | If a request path matches more than one parameterized routes, the one with longer static prefix will be returned. |
|   Wildcard    |    3     |     `/api/v1/#any`      | A name parameter follow by `#`. Note that wildcard must be the last segment of the path. |                                                                                                                   |


#### Path Parameter

Use `http_request::path()` to access the route parameters. ([Code](https://github.com/Ramirisu/fitoria/blob/main/example/web/path_parameter.cpp))

```cpp

namespace api::v1::users::get_user {
auto api(const http_request& req) -> net::awaitable<http_response>
{
  auto user = req.path().get("user");
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
                    .serve(route::get<"/api/v1/users/{user}">(
                        api::v1::users::get_user::api))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Query String Parameters

Use `http_request::query()` to access the query string parameters. ([Code](https://github.com/Ramirisu/fitoria/blob/main/example/web/query_string.cpp))

```cpp

namespace api::v1::users {
auto get_user(const http_request& req) -> net::awaitable<http_response>
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
            .serve(route::get<"/api/v1/users">(api::v1::users::get_user))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Urlencoded Post Form

Use `as_form()` to parse the url-encoded form body. ([Code](https://github.com/Ramirisu/fitoria/blob/main/example/web/form.cpp))

```cpp

namespace api::v1::login {
auto api(const http_request& req, std::string body) -> net::awaitable<http_response>
{
  if (req.fields().get(http::field::content_type)
      != http::fields::content_type::form_urlencoded()) {
    co_return http_response(http::status::bad_request);
  }
  auto user = as_form(body);
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
                    .serve(route::post<"/api/v1/login">(api::v1::login::api))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Shared States

Configure shared states by using `scope::share_state(SharedState&&)` for the `route`s under the same `scope`, or `route::share_state(SharedState&&)` for the `route` itself. ([Code](https://github.com/Ramirisu/fitoria/blob/main/example/web/state.cpp))

```cpp

namespace cache {
class simple_cache {
  using map_type = unordered_string_map<std::string>;

public:
  optional<map_type::mapped_type> get(std::string_view key) const
  {
    auto lock = std::shared_lock { mutex_ };
    if (auto it = map_.find(key); it != map_.end()) {
      return it->second;
    }

    return nullopt;
  }

  bool put(const std::string& key, std::string value)
  {
    auto lock = std::unique_lock { mutex_ };
    return map_.insert_or_assign(key, std::move(value)).second;
  }

private:
  map_type map_;
  mutable std::shared_mutex mutex_;
};

using simple_cache_ptr = std::shared_ptr<simple_cache>;

auto put(const http_request& req) -> net::awaitable<http_response>
{
  auto key = req.params().get("key");
  auto value = req.params().get("value");
  if (!key || !value) {
    co_return http_response(http::status::bad_request);
  }

  auto cache = req.state<simple_cache_ptr>();
  if (!cache) {
    co_return http_response(http::status::internal_server_error);
  }

  if ((*cache)->put(*key, *value)) {
    co_return http_response(http::status::created);
  } else {
    co_return http_response(http::status::accepted);
  }
}

auto get(const http_request& req) -> net::awaitable<http_response>
{
  auto key = req.params().get("key");
  if (!key) {
    co_return http_response(http::status::bad_request);
  }

  auto cache = req.state<simple_cache_ptr>();
  if (!cache) {
    co_return http_response(http::status::internal_server_error);
  }

  if (auto value = (*cache)->get(*key); value) {
    co_return http_response(http::status::ok)
        .set_field(http::field::content_type,
                   http::fields::content_type::plaintext())
        .set_body(*value);
  } else {
    co_return http_response(http::status::not_found);
  }
}

}

int main()
{
  auto cache = std::make_shared<cache::simple_cache_ptr>();

  auto server = http_server::builder()
                    .serve(scope<"/cache">()
                               .share_state(cache)
                               .serve(route::put<"/{key}/{value}">(cache::put))
                               .serve(route::get<"/{key}">(cache::get)))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Extractor

Extractors provide a more convenient way to help user access information from `http_request`. Users can specify as many extractors as compiler allows per handler. ([Code](https://github.com/Ramirisu/fitoria/blob/main/example/web/extractor.cpp))

Built-in Extractors:

| Extractor                | Description                                            | Body Extractor |                                                                                                                                                                       |
| :----------------------- | :----------------------------------------------------- | :------------: | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `web::http_request`      | Extract whole `http_request`                           |       no       |                                                                                                                                                                       |
| `web::connection_info`   | Extract connection info                                |       no       |                                                                                                                                                                       |
| `web::path_info`         | Extract path info parameter                            |       no       |                                                                                                                                                                       |
| `web::path<T>`           | Extract path parameter into type `T`.                  |       no       | `T = std::tuple<Ts...>`, parameters are extracted in the order where they are in the path.<br/> `T = aggregate`, parameters are extracted to the field of their name. |
| `web::query_map`         | Extract query string parameters                        |       no       |                                                                                                                                                                       |
| `web::query<T>`          | Extract query string parameters into type `T`          |       no       | `T = aggregate`, parameters are extracted to the field of their name.                                                                                                 |
| `web::http_fields`       | Extract fields from request headers                    |       no       |                                                                                                                                                                       |
| `web::state<T>`          | Extract shared state of type `T`.                      |       no       | Note that unlike `http_request::state<T>()` which returns `optional<T&>`, extractor ***copy the value***.                                                             |
| `std::string`            | Extract body as `std::string`                          |      yes       |                                                                                                                                                                       |
| `std::vector<std::byte>` | Extract body as `std::vector<std::byte>`               |      yes       |                                                                                                                                                                       |
| `web::json<T>`           | Extract body and parse it into json and convert to `T` |      yes       |                                                                                                                                                                       |

> Implement `from_http_request_t` CPO to define custom extractors.

> The ***body extractor*** can only be used at most once in the request handlers since it consumes the body.

```cpp

namespace api::v1::login {
struct secret_t {
  std::string password;
};

boost::json::result_for<secret_t, boost::json::value>::type
tag_invoke(const boost::json::try_value_to_tag<secret_t>&,
           const boost::json::value& jv)
{
  secret_t user;

  if (!jv.is_object()) {
    return make_error_code(boost::json::error::incomplete);
  }

  const auto& obj = jv.get_object();

  auto* password = obj.if_contains("password");
  if (password && password->is_string()) {
    return secret_t { .password = std::string(password->get_string()) };
  }

  return make_error_code(boost::json::error::incomplete);
}

auto api(const connection_info& conn_info,
         const path_info& path_info,
         state<database_ptr> db,
         json<secret_t> secret) -> net::awaitable<http_response>
{
  std::cout << fmt::format("client addr {}:{}\n",
                           conn_info.remote_addr().to_string(),
                           conn_info.remote_port());
  if (secret.password
      == path_info.get("user").and_then([&](auto&& name) -> optional<std::string> {
           if (auto it = db->find(name); it != db->end()) {
             return it->second;
           }
           return nullopt;
         })) {
    co_return http_response(http::status::ok)
        .set_field(http::field::content_type,
                   http::fields::content_type::plaintext())
        .set_body("login succeeded");
  }
  co_return http_response(http::status::unauthorized)
      .set_field(http::field::content_type,
                 http::fields::content_type::plaintext())
      .set_body("user name or password is incorrect");
}
}

int main()
{
  auto db = std::make_shared<database_t>();
  db->insert({ "albert", "123456" });

  auto server
      = http_server::builder()
            .serve(route::post<"/api/v1/login/{user}">(api::v1::login::api)
                       .share_state(db))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}


```

#### Scope

Grouping `route`s by `scope`. ([Code](https://github.com/Ramirisu/fitoria/blob/main/example/web/scope.cpp))

```cpp

int main()
{
  auto server
      = http_server::builder()
            .serve(
                scope<>()
                    .use(middleware::logger())
                    .serve(scope<"/api/v1">()
                               .serve(route::post<"/register">(api::v1::reg))
                               .serve(route::post<"/login">(api::v1::login)))
                    .serve(scope<"/api/v2">()
                               .serve(route::post<"/register">(api::v2::reg))
                               .serve(route::post<"/login">(api::v2::login))))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Middleware

Use `scope::use(Service&&)` to configure middlewares for its `router`s. ([Code](https://github.com/Ramirisu/fitoria/blob/main/example/web/middleware.cpp))

fitoria provides following build-in middlewares:

* `middleware::logger`
* `middleware::exception_handler`
* `middleware::deflate`
* `middleware::gzip`

> Implement `new_middleware_t` CPO to define custom middlewares.

```cpp

template <typename Next>
class my_log_middleware {
  friend class my_log;

public:
  auto operator()(http_context& ctx) const -> net::awaitable<http_response>
  {
    log::log(lv_, "before handler");

    auto res = co_await next_(ctx);

    log::log(lv_, "after handler");

    co_return res;
  }

private:
  my_log_middleware(Next next, log::level lv)
      : next_(std::move(next))
      , lv_(lv)
  {
  }

  Next next_;
  log::level lv_;
};

class my_log {
public:
  my_log(log::level lv)
      : lv_(lv)
  {
  }

  template <uncvref_same_as<my_log> Self, typename Next>
  friend auto tag_invoke(new_middleware_t, Self&& self, Next&& next)
  {
    return std::forward<Self>(self).new_middleware_impl(
        std::forward<Next>(next));
  }

private:
  template <typename Next>
  auto new_middleware_impl(Next&& next) const
  {
    return my_log_middleware(std::move(next), lv_);
  }

  log::level lv_;
};

auto get_user(http_request& req) -> net::awaitable<http_response>
{
  log::debug("user: {}", req.params().get("user"));

  co_return http_response(http::status::ok)
      .set_field(http::field::content_type,
                 http::fields::content_type::plaintext())
      .set_body(req.params().get("user").value_or("{{unknown}}"));
}

int main()
{
  log::global_logger() = log::stdout_logger();
  log::global_logger()->set_log_level(log::level::debug);

  auto server = http_server::builder()
                    .serve(scope<"/api/v1">()
                               .use(middleware::logger())
                               .use(middleware::exception_handler())
                               .use(my_log(log::level::info))
                               .serve(route::get<"/users/{user}">(get_user)))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}

```

#### Graceful Shutdown

Use `net::signal_set` to handle signals to shutdown the server gracefully. ([Code](https://github.com/Ramirisu/fitoria/blob/main/example/web/graceful_shutdown.cpp))

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

#### WebSockets

***TODO:***

#### Unit Testing

`http::async_serve_request()` can consume the `http_request` directly without creating TCP connections. ([Code](https://github.com/Ramirisu/fitoria/blob/main/example/web/unittesting.cpp))

```cpp

int main()
{
  auto server
      = http_server::builder()
            .serve(route::post<"/api/v1/login">(
                [](http_request& req, std::string body) -> net::awaitable<http_response> {
                  if (req.fields().get(http::field::content_type)
                      != http::fields::content_type::form_urlencoded()) {
                    co_return http_response(http::status::bad_request);
                  }
                  auto user = as_form(body);
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

  net::sync_wait([&]() -> net::awaitable<void> {
    {
      auto res = co_await server.async_serve_request(
          "/api/v1/login",
          http_request(http::verb::post)
              .set_field(http::field::content_type,
                         http::fields::content_type::plaintext())
              .set_body("name=ramirisu&password=123456"));
      FITORIA_ASSERT(res.status_code() == http::status::bad_request);
    }
    {
      auto res = co_await server.async_serve_request(
          "/api/v1/login",
          http_request(http::verb::post)
              .set_field(http::field::content_type,
                         http::fields::content_type::form_urlencoded())
              .set_body("name=unknown&password=123"));
      FITORIA_ASSERT(res.status_code() == http::status::unauthorized);
    }
    {
      auto res = co_await server.async_serve_request(
          "/api/v1/login",
          http_request(http::verb::post)
              .set_field(http::field::content_type,
                         http::fields::content_type::form_urlencoded())
              .set_body("name=ramirisu&password=123"));
      FITORIA_ASSERT(res.status_code() == http::status::unauthorized);
    }
    {
      auto res = co_await server.async_serve_request(
          "/api/v1/login",
          http_request(http::verb::post)
              .set_field(http::field::content_type,
                         http::fields::content_type::form_urlencoded())
              .set_body("name=ramirisu&password=123456"));
      FITORIA_ASSERT(res.status_code() == http::status::ok);
      FITORIA_ASSERT(res.fields().get(http::field::content_type)
                     == http::fields::content_type::plaintext());
      FITORIA_ASSERT((co_await res.as_string()) == "ramirisu, login succeeded");
    }
  }());
}

```

### HTTP Client

***TODO:***

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

|    Library     | Usage                              |    Namespace    | required/optional |
| :------------: | :--------------------------------- | :-------------: | :---------------: |
| `boost::asio`  | Networking                         | `fitoria::net`  |     required      |
| `boost::beast` | HTTP                               | `fitoria::http` |     required      |
|  `boost::url`  | Url parsing                        |                 |     required      |
| `boost::json`  | JSON serialization/deserialization |                 |     required      |
| `boost::regex` | Route parsing                      |                 |     required      |
|  `boost::pfr`  | `web::path<T>` extractor           |                 |     optional      |
|     `zlib`     | Built-in middleware gzip           |                 |     optional      |
|     `fmt`      | Formatting                         | `fitoria::fmt`  |     optional      |
|   `OpenSSL`    | Secure networking                  |                 |     optional      |
|   `doctest`    | Unit testing                       |                 |     optional      |

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
