//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

namespace api::v1::login {
struct secret_t {
  std::string password;
};

boost::json::result_for<secret_t, boost::json::value>::type
tag_invoke(const boost::json::try_value_to_tag<secret_t>&,
           const boost::json::value& jv)
{
  secret_t user;

  if (!jv.is_object()) {
    return make_error_code(boost::json::error::incomplete);
  }

  const auto& obj = jv.get_object();

  auto* password = obj.if_contains("password");
  if (password && password->is_string()) {
    return secret_t { .password = std::string(password->get_string()) };
  }

  return make_error_code(boost::json::error::incomplete);
}

auto api(const route_params& params, json<secret_t> secret)
    -> lazy<http_response>
{
  if (params.get("user") != "ramirisu" || secret.password != "123456") {
    co_return http_response(http::status::unauthorized)
        .set_body("user name or password is incorrect");
  }
  co_return http_response(http::status::ok).set_body("login succeeded");
}
}

int main()
{
  auto server
      = http_server::builder()
            .route(route::POST<"/api/v1/login/{user}">(api::v1::login::api))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
