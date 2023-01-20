//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;

// clang-format off
//
// $ ./form
// $ curl -X POST http://127.0.0.1:8080/api/v1/login -d 'name=ramirisu&password=123456' --verbose
//
// curl output:
// < HTTP/1.1 200 OK
// < Content-Type: text/plain
// < Content-Length: 0
// 
//
// clang-format on

int main()
{
  auto server
      = http_server::builder()
            .route(router(
                http::verb::post, "/api/v1/login",
                [](const http_request& req) -> net::awaitable<http_response> {
                  if (req.headers().get(http::field::content_type)
                      != "application/x-www-form-urlencoded") {
                    co_return http_response(http::status::bad_request);
                  }
                  auto user = as_form(req.body());
                  if (!user || user->get("name") != "ramirisu"
                      || user->get("password") != "123456") {
                    co_return http_response(http::status::unauthorized);
                  }

                  co_return http_response(http::status::ok);
                }))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
