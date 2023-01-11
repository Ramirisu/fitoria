//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;

int main()
{
  log::global_logger() = log::stdout_logger();
  log::global_logger()->set_log_level(log::level::debug);

  auto server
      = http_server::builder()
            // Extract `http_request`
            .route(
                router(http::verb::put, "/api/v1/users/{user}",
                       [](http_request& req) -> net::awaitable<http_response> {
                         log::debug("{}", req.route().path());

                         co_return http_response(http::status::ok);
                       }))
            // Extract `http_route` only
            .route(router(
                http::verb::post, "/api/v1/services/{service}",
                [](const http_route& r) -> net::awaitable<http_response> {
                  log::debug("{}", r.path());

                  co_return http_response(http::status::ok);
                }))
            // Extract `query_map` only
            .route(
                router(http::verb::get, "/api/v1/games/{game}",
                       [](query_map& query) -> net::awaitable<http_response> {
                         log::debug("{}", query.get("name"));

                         co_return http_response(http::status::ok);
                       }))
            // Extract arbitrary type as you desired
            .route(router(
                http::verb::patch, "/api/v1/languages/{language}",
                [](const http_route& r, http_request& req,
                   const query_map& query) -> net::awaitable<http_response> {
                  log::debug("{}", req.route().path());
                  log::debug("{}", r.path());
                  log::debug("{}", query.get("name"));

                  co_return http_response(http::status::ok);
                }))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();
}
