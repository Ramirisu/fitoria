//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;
using namespace fitoria::web;

// clang-format off
//
// $ ./form
// $ curl -X POST http://127.0.0.1:8080/api/v1/login -d 'name=ramirisu&password=123456' -v
//
// curl output:
// < HTTP/1.1 200 OK
// < Content-Type: text/plain
// < Content-Length: 0
// 
//
// clang-format on

namespace api::v1::login {
auto api(const http_request& req) -> lazy<http_response>
{
  if (req.fields().get(http::field::content_type)
      != http::fields::content_type::form_urlencoded()) {
    co_return http_response(http::status::bad_request);
  }
  auto user = as_form(req.body());
  if (!user || user->get("name") != "ramirisu"
      || user->get("password") != "123456") {
    co_return http_response(http::status::unauthorized);
  }

  co_return http_response(http::status::ok);
}
}

int main()
{
  auto server = http_server::builder()
                    .route(route::POST("/api/v1/login", api::v1::login::api))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
