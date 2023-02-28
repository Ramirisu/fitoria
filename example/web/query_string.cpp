//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;
using namespace fitoria::web;

// $ ./query_string
// $ curl -X GET http://127.0.0.1:8080/api/v1/users?user=ramirisu -v
//
// curl output:
// < HTTP/1.1 200 OK
// < Content-Type: text/plain
// < Content-Length: 14
// <
// user: ramirisu

namespace api::v1::users::get_user {
auto api(const http_request& req) -> lazy<http_response>
{
  auto user = req.query().get("user");
  if (!user) {
    co_return http_response(http::status::bad_request);
  }

  co_return http_response(http::status::ok)
      .set_plaintext(fmt::format("user: {}", user.value()));
}
}

int main()
{
  auto server
      = http_server::builder()
            .route(route::GET<"/api/v1/users">(api::v1::users::get_user::api))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
