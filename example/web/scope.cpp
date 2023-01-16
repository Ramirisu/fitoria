//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;

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
      // Global router group
      scope("")
          // Register a global middleware for all handlers
          .use(my_middleware::log)
          // Create a subgroup "/api/v1" under global router group
          .sub_group(
              scope("/api/v1")
                  // Register a middleware for this group
                  .use(my_middleware::v1::auth)
                  // Register a route for this group
                  .route(router(http::verb::get, "/users/{user}",
                                []([[maybe_unused]] http_request& req)
                                    -> net::awaitable<http_response> {
                                  log::debug("route: {}", req.route().path());

                                  co_return http_response(http::status::ok);
                                })))
          // Create a subgroup "/api/v2" under global router group
          .sub_group(
              scope("/api/v2")
                  // Register a middleware for this group
                  .use(my_middleware::v2::auth)
                  // Register a route for this group
                  .route(router(http::verb::get, "/users/{user}",
                                []([[maybe_unused]] http_request& req)
                                    -> net::awaitable<http_response> {
                                  log::debug("route: {}", req.route().path());

                                  co_return http_response(http::status::ok);
                                }))));
}

// clang-format off
// 
// $ ./scope
// $ curl -X GET http://127.0.0.1:8080/api/v1/users/ramirisu --verbose
//
// > 2023-01-01T00:00:00Z DEBUG log middleware (in) [scope.cpp:16:13]
// > 2023-01-01T00:00:00Z DEBUG v1 auth middleware (in) [scope.cpp:25:15]
// > 2023-01-01T00:00:00Z DEBUG route: /api/v1/users/{user} [scope.cpp:59:45]
// > 2023-01-01T00:00:00Z DEBUG v1 auth middleware (out) [scope.cpp:27:15]
// > 2023-01-01T00:00:00Z DEBUG log middleware (out) [scope.cpp:18:13]
//
// $ curl -X GET http://127.0.0.1:8080/api/v2/users/ramirisu --verbose
//
// > 2023-01-01T00:00:00Z DEBUG log middleware (in) [scope.cpp:16:13]
// > 2023-01-01T00:00:00Z DEBUG v2 auth middleware (in) [scope.cpp:25:15]
// > 2023-01-01T00:00:00Z DEBUG route: /api/v2/users/{user} [scope.cpp:59:45]
// > 2023-01-01T00:00:00Z DEBUG v2 auth middleware (out) [scope.cpp:27:15]
// > 2023-01-01T00:00:00Z DEBUG log middleware (out) [scope.cpp:18:13]
// 
//
// clang-format on

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

  server.wait();
}
