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
// $ ./json
// $ curl -X POST http://127.0.0.1:8080/api/v1/login -H 'Content-Type: application/json' -d '{"name":"ramirisu","password":"123456"}' --verbose
//
// curl output:
// < HTTP/1.1 200 OK
// < Content-Type: application/json
// < Content-Length: 25
// <
// {"msg":"login succeeded"}
// 
//
// clang-format on

struct user_t {
  std::string name;
  std::string password;

  friend bool operator==(const user_t&, const user_t&) = default;
};

user_t tag_invoke(const json::value_to_tag<user_t>&, const json::value& jv)
{
  return user_t {
    .name = std::string(jv.at("name").as_string()),
    .password = std::string(jv.at("password").as_string()),
  };
}

void tag_invoke(const json::value_from_tag&,
                json::value& jv,
                const user_t& user)
{
  jv = {
    { "name", user.name },
    { "password", user.password },
  };
}

int main()
{
  auto server
      = http_server::builder()
            .route(router(
                http::verb::post, "/api/v1/login",
                [](const http_request& req) -> net::awaitable<http_response> {
                  auto user = req.body_as_json<user_t>();
                  if (!user) {
                    co_return http_response(http::status::bad_request)
                        .set_json({ { "msg", user.error().message() } });
                  }
                  if (user->name != "ramirisu" || user->password != "123456") {
                    co_return http_response(http::status::unauthorized)
                        .set_json({ { "msg",
                                      "user name or password is incorrect" } });
                  }
                  co_return http_response(http::status::ok)
                      .set_json({ { "msg", "login succeeded" } });
                }))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
