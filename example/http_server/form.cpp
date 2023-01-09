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
  auto server = http_server(http_server_config().route(
      router(http::verb::post, "/api/v1/login",
             [](const http_request& req) -> net::awaitable<http_response> {
               auto user = req.body_as_form();
               if (!user || user->get("name") != "ramirisu"
                   || user->get("password") != "123456") {
                 co_return http_response(http::status::bad_request);
               }

               co_return http_response(http::status::ok);
             })));
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();
}
