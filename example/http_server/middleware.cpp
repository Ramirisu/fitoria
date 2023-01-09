//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/http_server.hpp>
#include <fitoria/http_server/middleware/logger.hpp>

#include <iostream>

using namespace fitoria;

// $ ./middleware
// $ curl -X GET http://127.0.0.1:8080/api/v1/users/ramirisu
//
// clang-format off
// > 2023-01-09T06:45:17Z DEBUG before handler [middleware.cpp:36:23]
// > 2023-01-09T06:45:17Z DEBUG user: ramirisu [middleware.cpp:47:37]
// > 2023-01-09T06:45:17Z DEBUG after handler [middleware.cpp:38:23]
// > 2023-01-09T06:45:17Z INFO [fitoria.middleware.logger] 127.0.0.1 GET /api/v1/users/ramirisu 200 curl/7.83.1 [logger.hpp:31:14]
// clang-format on

int main()
{
  log::global_logger()->set_log_level(log::level::debug);

  auto server = http_server(http_server_config().route(
      // Add a router group
      router_group("/api/v1")
          // Register built-in logger middleware fot this group
          .use(middleware::logger())
          // Register a custom middleware for this group
          .use([](http_context& c) -> net::awaitable<http_response> {
            log::debug("before handler");
            auto res = co_await c.next();
            log::debug("after handler");
            co_return res;
          })
          // Register a route for this group
          // The route is associated with the middleware defined above
          .route(router(http::verb::get, "/users/{user}",
                        [](http_request& req) -> net::awaitable<http_response> {
                          FITORIA_ASSERT(req.method() == http::verb::get);

                          log::debug("user: {}", req.route().get("user"));

                          co_return http_response(http::status::ok);
                        }))));
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();
}
