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
// $ ./extractor
// 
// # Query user last login time
// $ curl -X GET http://localhost:8080/api/v1/users/albert -v
// 
// # Login
// #### linux
// $ curl -X POST http://localhost:8080/api/v1/login -H 'Content-Type: application/json' -d '{ "username": "albert", "password": "123456" }' -v
// #### windows
// $ curl -X POST http://localhost:8080/api/v1/login -H "Content-Type: application/json" -d '{ \"username\": \"albert\", \"password\": \"123456\" }' -v
//

// clang-format on

namespace database {

using clock_t = std::chrono::system_clock;
using time_point = std::chrono::time_point<clock_t>;

struct user_t {
  std::string password;
  optional<time_point> last_login_time;
};

using type = std::unordered_map<std::string, user_t>;
using ptr = std::shared_ptr<type>;
}

namespace api::v1 {
namespace users {
  auto api(path_of<std::tuple<std::string>> path, state_of<database::ptr> db)
      -> net::awaitable<http_response>
  {
    auto [user] = std::move(path);
    if (auto it = db->find(user); it != db->end()) {
      if (it->second.last_login_time) {
        co_return http_response(http::status::ok)
            .set_field(http::field::content_type,
                       http::fields::content_type::plaintext())
            .set_body(fmt::format("{:%FT%TZ}", *(it->second.last_login_time)));
      } else {
        co_return http_response(http::status::internal_server_error)
            .set_field(http::field::content_type,
                       http::fields::content_type::plaintext())
            .set_body(fmt::format("User [{}] never logins.", user));
      }
    }
    co_return http_response(http::status::not_found)
        .set_field(http::field::content_type,
                   http::fields::content_type::plaintext())
        .set_body("User does not exist.");
  }
}
namespace login {
  struct body_type {
    std::string username;
    std::string password;
  };

  boost::json::result_for<body_type, boost::json::value>::type
  tag_invoke(const boost::json::try_value_to_tag<body_type>&,
             const boost::json::value& jv)
  {
    if (!jv.is_object()) {
      return make_error_code(boost::json::error::incomplete);
    }

    const auto& obj = jv.get_object();

    auto* username = obj.if_contains("username");
    auto* password = obj.if_contains("password");
    if (!username || !username->is_string() || !password
        || !password->is_string()) {
      return make_error_code(boost::json::error::incomplete);
    }
    return body_type { .username = std::string(username->get_string()),
                       .password = std::string(password->get_string()) };
  }

  auto api(state_of<database::ptr> db, json_of<body_type> body)
      -> net::awaitable<http_response>
  {
    if (auto it = db->find(body.username);
        it != db->end() && it->second.password == body.password) {
      it->second.last_login_time = database::clock_t::now();
      co_return http_response(http::status::ok)
          .set_field(http::field::content_type,
                     http::fields::content_type::plaintext())
          .set_body("Login succeeded.");
    }
    co_return http_response(http::status::unauthorized)
        .set_field(http::field::content_type,
                   http::fields::content_type::plaintext())
        .set_body("User name or password is incorrect.");
  }
}
}

int main()
{
  auto db = std::make_shared<database::type>();
  db->insert({ "albert",
               database::user_t { .password = "123456",
                                  .last_login_time = nullopt } });

  auto server
      = http_server::builder()
            .serve(route::get<"/api/v1/users/{user}">(api::v1::users::api)
                       .state(db))
            .serve(route::post<"/api/v1/login">(api::v1::login::api).state(db))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
