//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

// $ ./route_parameter
// $ curl -X GET http://127.0.0.1:8080/api/v1/users/david -v
//
// curl output:
// < HTTP/1.1 200 OK
// < Content-Type: text/plain
// < Content-Length: 14
// <
// user: david

auto get_user(const request& req) -> awaitable<response>
{
  auto user = req.path().get("user");
  if (!user) {
    co_return response::bad_request().build();
  }

  co_return response::ok()
      .set_header(http::field::content_type, mime::text_plain())
      .set_body(fmt::format("user: {}", user.value()));
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/api/v1/users/{user}">(get_user))
                    .build();
  server.bind("127.0.0.1", 8080);

  ioc.run();
}
