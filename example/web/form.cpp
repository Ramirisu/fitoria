//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

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
auto api(const request& req, std::string body) -> awaitable<response>
{
  if (req.fields().get(http::field::content_type)
      != http::fields::content_type::form_urlencoded()) {
    co_return response::bad_request().build();
  }
  auto user = as_form(body);
  if (!user || user->get("name") != "ramirisu"
      || user->get("password") != "123456") {
    co_return response::unauthorized().build();
  }

  co_return response::ok().build();
}
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server_builder(ioc)
                    .serve(route::post<"/api/v1/login">(api::v1::login::api))
                    .build();
  server.bind("127.0.0.1", 8080);

  ioc.run();
}
