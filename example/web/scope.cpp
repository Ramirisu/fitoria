//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;
using namespace fitoria::web;

int main()
{
  log::global_logger() = log::stdout_logger();
  log::global_logger()->set_log_level(log::level::debug);

  auto server
      = http_server::builder()
            // Use a configure function to setup server configuration
            .route(
                // Global scope
                scope<"">()
                    // Register a global middleware for all handlers
                    .use(middleware::logger())
                    // Create a sub-scope "/api/v1" under global scope
                    .sub_scope(
                        scope<"/api/v1">()
                            // Register a middleware for this scope
                            .use(middleware::gzip())
                            // Register a route for this scope
                            .GET<"/users/{user}">(
                                [](http_request& req) -> lazy<http_response> {
                                  log::debug("route: {}", req.params().path());

                                  co_return http_response(http::status::ok);
                                }))
                    // Create a sub-scope "/api/v2" under global scope
                    .sub_scope(
                        scope<"/api/v2">()
                            // Register a middleware for this scope
                            .use(middleware::deflate())
                            // Register a route for this scope
                            .GET<"/users/{user}">(
                                [](http_request& req) -> lazy<http_response> {
                                  log::debug("params: {}", req.params().path());

                                  co_return http_response(http::status::ok);
                                })))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
