//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/http_server.hpp>

#include <iostream>

using namespace fitoria;

int main()
{
  auto server = http_server(
      http_server_config()
          .route(router(http::verb::put, "/api/v1/users/{user}",
                        [](http_request& req) -> net::awaitable<http_response> {
                          std::cout << req.route().path() << "\n";

                          co_return http_response(http::status::ok);
                        }))
          .route(router(http::verb::post, "/api/v1/services/{service}",
                        [](http_route& r) -> net::awaitable<http_response> {
                          std::cout << r.path() << "\n";

                          co_return http_response(http::status::ok);
                        }))
          .route(router(http::verb::get, "/api/v1/games/{game}",
                        [](query_map& query) -> net::awaitable<http_response> {
                          std::cout << query.get("name").value() << "\n";

                          co_return http_response(http::status::ok);
                        }))
          .route(router(http::verb::patch, "/api/v1/languages/{language}",
                        [](http_route& r,
                           http_request& req) -> net::awaitable<http_response> {
                          std::cout << r.path() << "\n";
                          std::cout << req.route().path() << "\n";

                          co_return http_response(http::status::ok);
                        })));
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();
}
