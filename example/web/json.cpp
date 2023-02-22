//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

// clang-format off
//
// $ ./json
// $ curl -X POST http://127.0.0.1:8080/api/v1/login -H 'Content-Type: application/json' -d '{"name":"ramirisu","password":"123456"}' -v
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

namespace api::v1::login {
struct user_t {
  std::string name;
  std::string password;
};

boost::json::result_for<user_t, boost::json::value>::type
tag_invoke(const boost::json::try_value_to_tag<user_t>&,
           const boost::json::value& jv)
{
  user_t user;

  if (!jv.is_object()) {
    return make_error_code(boost::json::error::incomplete);
  }

  const auto& obj = jv.get_object();

  auto* name = obj.if_contains("name");
  auto* password = obj.if_contains("password");
  if (name && password && name->is_string() && password->is_string()) {
    return user_t { .name = std::string(name->get_string()),
                    .password = std::string(password->get_string()) };
  }

  return make_error_code(boost::json::error::incomplete);
}

struct output {
  std::string msg;
};

void tag_invoke(const boost::json::value_from_tag&,
                boost::json::value& jv,
                const output& out)
{
  jv = { { "msg", out.msg } };
}

auto api(extractor::json<user_t> user) -> lazy<http_response>
{
  if (user.name != "ramirisu" || user.password != "123456") {
    co_return http_response(http::status::unauthorized)
        .set_json(output { .msg = "user name or password is incorrect" });
  }
  co_return http_response(http::status::ok)
      .set_json(output { .msg = "login succeeded" });
}
}

int main()
{
  auto server = http_server::builder()
                    .route(route::POST<"/api/v1/login">(api::v1::login::api))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
