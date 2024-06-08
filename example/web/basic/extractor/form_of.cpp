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
// $ curl -X POST http://127.0.0.1:8080/api/v1/login -d 'username=fitoria&password=123456' -v
//
// > POST /api/v1/login HTTP/1.1
// > Host: 127.0.0.1:8080
// > User-Agent: curl/8.4.0
// > Accept: */*
// > Content-Length: 32
// > Content-Type: application/x-www-form-urlencoded
// >
// < HTTP/1.1 200 OK
// < Content-Length: 0
// <
// 
//
// clang-format on

namespace api::v1::login {

struct user_t {
  std::string username;
  std::string password;
};

auto api(form_of<user_t> user) -> awaitable<response>
{
  if (user.username != "fitoria" || user.password != "123456") {
    co_return response::unauthorized().build();
  }

  co_return response::ok().build();
}
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::post<"/api/v1/login">(api::v1::login::api))
                    .build();
  server.bind("127.0.0.1", 8080);

  ioc.run();
}
