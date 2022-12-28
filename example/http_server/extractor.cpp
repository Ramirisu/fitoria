//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/http_server.hpp>

using namespace fitoria;

int main()
{
  auto server = http_server(
      http_server_config()
          .route(router(
              verb::put, "/api/v1/users/{user}",
              [](http_request& req)
                  -> net::awaitable<expected<http_response, http_error>> {
                FITORIA_ASSERT(req.method() == verb::put);
                co_return http_response(status::ok);
              }))
          .route(router(
              verb::post, "/api/v1/services/{service}",
              [](http_route& r)
                  -> net::awaitable<expected<http_response, http_error>> {
                FITORIA_ASSERT(r.path() == "/api/v1/services/{service}");
                co_return http_response(status::ok);
              }))
          .route(router(
              verb::patch, "/api/v1/languages/{language}",
              [](http_route& r, http_request& req)
                  -> net::awaitable<expected<http_response, http_error>> {
                FITORIA_ASSERT(r.path() == "/api/v1/languages/{language}");
                FITORIA_ASSERT(req.method() == verb::patch);
                co_return http_response(status::ok);
              })));
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();
}
