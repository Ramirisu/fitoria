//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/http_server/http_server.hpp>

using namespace fitoria;

int main()
{
  auto server = http_server(
      http_server_config()
          .route(router(verb::get, "/api/v1/{owner}/{repo}",
                        [](http_context& c) -> net::awaitable<void> {
                          FITORIA_ASSERT(c.route().path()
                                         == "/api/v1/{owner}/{repo}");
                          FITORIA_ASSERT(c.request().method() == verb::get);
                          co_return;
                        }))
          .route(router(verb::post, "/api/v1/services/{service}",
                        [](http_route& r) -> net::awaitable<void> {
                          FITORIA_ASSERT(r.path()
                                         == "/api/v1/services/{service}");
                          co_return;
                        }))
          .route(router(verb::put, "/api/v1/users/{user}",
                        [](http_request& req) -> net::awaitable<void> {
                          FITORIA_ASSERT(req.method() == verb::put);
                          co_return;
                        }))
          .route(router(
              verb::patch, "/api/v1/languages/{language}",
              [](http_route& r, http_request& req) -> net::awaitable<void> {
                FITORIA_ASSERT(r.path() == "/api/v1/languages/{language}");
                FITORIA_ASSERT(req.method() == verb::patch);
                co_return;
              })));
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();
}
