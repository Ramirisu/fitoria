//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/http_server.hpp>

#include <iostream>

using namespace fitoria;

namespace middleware {

auto log(http_context& c) -> net::awaitable<http_response>
{
  std::cout << "log middleware: request\n";
  auto res = co_await c.next();
  std::cout << "log middleware: response\n";
  co_return res;
}

namespace v1 {
  auto auth(http_context& c) -> net::awaitable<http_response>
  {
    std::cout << "auth middleware: /api/v1 request\n";
    auto res = co_await c.next();
    std::cout << "auth middleware: /api/v1 response\n";
    co_return res;
  }
}

namespace v2 {
  auto auth(http_context& c) -> net::awaitable<http_response>
  {
    std::cout << "auth middleware: /api/v2 request\n";
    auto res = co_await c.next();
    std::cout << "auth middleware: /api/v2 response\n";
    co_return res;
  }
}
}

void configure_application(http_server_config& config)
{
  config.route(
      // global router group
      router_group("")
          // register a global middleware for all handlers
          .use(middleware::log)
          // create a subgroup "/api/v1" under global router group
          .sub_group(
              router_group("/api/v1")
                  // register a middleware for this group
                  .use(middleware::v1::auth)
                  // register a route for this group
                  .route(router(
                      http::verb::get, "/users/{user}",
                      [](http_request& req) -> net::awaitable<http_response> {
                        FITORIA_ASSERT(req.route().path()
                                       == "/api/v1/users/{user}");
                        co_return http_response(http::status::ok);
                      })))
          // create a subgroup "/api/v2" under global router group
          .sub_group(
              router_group("/api/v2")
                  // register a middleware for this group
                  .use(middleware::v2::auth)
                  // register a route for this group
                  .route(router(
                      http::verb::get, "/users/{user}",
                      [](http_request& req) -> net::awaitable<http_response> {
                        FITORIA_ASSERT(req.route().path()
                                       == "/api/v2/users/{user}");
                        co_return http_response(http::status::ok);
                      }))));
}

int main()
{
  auto server = http_server(
      http_server_config()
          // use a configure function to setup server configuration
          .configure(configure_application));
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();
}
