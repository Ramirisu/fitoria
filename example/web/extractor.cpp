//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

#include <iostream>

using namespace fitoria;
using namespace fitoria::web;

// clang-format off

// $ ./extractor
// 
// # linux
// $ curl -X POST http://localhost:8080/api/v1/login/unknown -H 'Content-Type: application/json' -d '{ "password": "123456" }' -v
// # windows
// $ curl -X POST http://localhost:8080/api/v1/login/unknown -H "Content-Type: application/json" -d '{ \"password\": \"123456\" }' -v
//
// curl output:
// *   Trying [::1]:8080...
// *   Trying 127.0.0.1:8080...
// * Connected to localhost (127.0.0.1) port 8080
// > POST /api/v1/login/unknown HTTP/1.1
// > Host: localhost:8080
// > User-Agent: curl/8.4.0
// > Accept: */*
// > Content-Type: application/json
// > Content-Length: 24
// >
// < HTTP/1.1 401 Unauthorized
// < Content-Type: text/plain; charset=utf-8
// < Content-Length: 34
// <
// user name or password is incorrect
//
// # linux
// $ curl -X POST http://localhost:8080/api/v1/login/albert -H 'Content-Type: application/json' -d '{ "password": "123456" }' -v
// # windows
// $ curl -X POST http://localhost:8080/api/v1/login/albert -H "Content-Type: application/json" -d '{ \"password\": \"123456\" }' -v
//
// curl output:
// *   Trying [::1]:8080...
// *   Trying 127.0.0.1:8080...
// * Connected to localhost (127.0.0.1) port 8080
// > POST /api/v1/login/albert HTTP/1.1
// > Host: localhost:8080
// > User-Agent: curl/8.4.0
// > Accept: */*
// > Content-Type: application/json
// > Content-Length: 24
// >
// < HTTP/1.1 200 OK
// < Content-Type: text/plain; charset=utf-8
// < Content-Length: 15
// <
// login succeeded

// clang-foramt on

using database_t = std::unordered_map<std::string, std::string>;
using database_ptr = std::shared_ptr<database_t>;

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

auto api(const connection_info& conn_info,
         const route_params& params,
         state<database_ptr> db,
         json<secret_t> secret) -> net::awaitable<http_response>
{
  std::cout << fmt::format("client addr {}:{}\n",
                           conn_info.remote_addr().to_string(),
                           conn_info.remote_port());
  if (secret.password
      == params.get("user").and_then([&](auto&& name) -> optional<std::string> {
           if (auto it = db->find(name); it != db->end()) {
             return it->second;
           }
           return nullopt;
         })) {
    co_return http_response(http::status::ok)
        .set_field(http::field::content_type,
                   http::fields::content_type::plaintext())
        .set_body("login succeeded");
  }
  co_return http_response(http::status::unauthorized)
      .set_field(http::field::content_type,
                 http::fields::content_type::plaintext())
      .set_body("user name or password is incorrect");
}
}

int main()
{
  auto db = std::make_shared<database_t>();
  db->insert({ "albert", "123456" });

  auto server
      = http_server::builder()
            .serve(route::post<"/api/v1/login/{user}">(api::v1::login::api)
                       .share_state(db))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
