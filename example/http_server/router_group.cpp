//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/http_server.hpp>

using namespace fitoria;

namespace middleware {

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

void configure_application(http_server_config& config)
{
  config.route(
      // Global router group
      router_group("")
          // Register a global middleware for all handlers
          .use(middleware::log)
          // Create a subgroup "/api/v1" under global router group
          .sub_group(
              router_group("/api/v1")
                  // Register a middleware for this group
                  .use(middleware::v1::auth)
                  // Register a route for this group
                  .route(router(http::verb::get, "/users/{user}",
                                []([[maybe_unused]] http_request& req)
                                    -> net::awaitable<http_response> {
                                  log::debug("route: {}", req.route().path());

                                  co_return http_response(http::status::ok);
                                })))
          // Create a subgroup "/api/v2" under global router group
          .sub_group(
              router_group("/api/v2")
                  // Register a middleware for this group
                  .use(middleware::v2::auth)
                  // Register a route for this group
                  .route(router(http::verb::get, "/users/{user}",
                                []([[maybe_unused]] http_request& req)
                                    -> net::awaitable<http_response> {
                                  log::debug("route: {}", req.route().path());

                                  co_return http_response(http::status::ok);
                                }))));
}

// $ ./router_group
// $ curl -X GET http://127.0.0.1:8080/api/v1/users/ramirisu
//
// clang-format off
// > 2023-01-01T00:00:00Z DEBUG log middleware (in) [router_group.cpp:16:13]
// > 2023-01-01T00:00:00Z DEBUG v1 auth middleware (in) [router_group.cpp:25:15]
// > 2023-01-01T00:00:00Z DEBUG route: /api/v1/users/{user} [router_group.cpp:59:45]
// > 2023-01-01T00:00:00Z DEBUG v1 auth middleware (out) [router_group.cpp:27:15]
// > 2023-01-01T00:00:00Z DEBUG log middleware (out) [router_group.cpp:18:13]
// clang-format on
//
// $ curl -X GET http://127.0.0.1:8080/api/v2/users/ramirisu
//
// clang-format off
// > 2023-01-01T00:00:00Z DEBUG log middleware (in) [router_group.cpp:16:13]
// > 2023-01-01T00:00:00Z DEBUG v2 auth middleware (in) [router_group.cpp:25:15]
// > 2023-01-01T00:00:00Z DEBUG route: /api/v2/users/{user} [router_group.cpp:59:45]
// > 2023-01-01T00:00:00Z DEBUG v2 auth middleware (out) [router_group.cpp:27:15]
// > 2023-01-01T00:00:00Z DEBUG log middleware (out) [router_group.cpp:18:13]
// clang-format on

int main()
{
  log::global_logger()->set_log_level(log::level::debug);

  auto server = http_server(
      http_server_config()
          // Use a configure function to setup server configuration
          .configure(configure_application));
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();
}
