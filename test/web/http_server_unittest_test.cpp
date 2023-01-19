//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("web.http_server.unittest");

namespace {

struct user_t {
  std::string name;
  std::string gender;
  std::string birth;
  std::string message;

  friend bool operator==(const user_t&, const user_t&) = default;
};

user_t tag_invoke(const json::value_to_tag<user_t>&, const json::value& jv)
{
  return user_t {
    .name = std::string(jv.at("name").as_string()),
    .gender = std::string(jv.at("gender").as_string()),
    .birth = std::string(jv.at("birth").as_string()),
    .message = std::string(jv.at("message").as_string()),
  };
}

void tag_invoke(const json::value_from_tag&,
                json::value& jv,
                const user_t& user)
{
  jv = {
    { "name", user.name },
    { "gender", user.gender },
    { "birth", user.birth },
    { "message", user.message },
  };
}

}

TEST_CASE("unittest")
{
  auto server
      = http_server::builder()
            .route(router(
                http::verb::get, "/api/v1/users/{user}",
                [](http_request& req) -> net::awaitable<http_response> {
                  user_t user;
                  user.name = req.route().get("user").value();
                  if (auto gender = req.query().get("gender"); gender) {
                    user.gender = gender.value();
                  } else {
                    co_return http_response(http::status::bad_request)
                        .set_json({ { "error", "gender is not provided" } });
                  }
                  if (auto birth = req.query().get("birth"); birth) {
                    user.birth = birth.value();
                  } else {
                    co_return http_response(http::status::bad_request)
                        .set_json({ { "error", "birth is not provided" } });
                  }
                  if (auto msg
                      = req.body_as_json()
                            .value_to_optional()
                            .and_then([](auto&& jv) -> optional<json::object> {
                              if (jv.is_object()) {
                                return jv.as_object();
                              }
                              return nullopt;
                            })
                            .and_then([](auto&& jo) -> optional<json::value> {
                              if (auto* jv = jo.if_contains("message");
                                  jv != nullptr) {
                                return *jv;
                              }
                              return nullopt;
                            })
                            .and_then([](auto&& jv) -> optional<std::string> {
                              if (jv.is_string()) {
                                return std::string(jv.as_string());
                              }
                              return nullopt;
                            });
                      msg) {
                    user.message = msg.value();
                  } else {
                    co_return http_response(http::status::bad_request)
                        .set_json({ { "error", "message is not provided" } });
                  }

                  co_return http_response(http::status::ok).set_json(user);
                }))
            .build();
  {
    auto res = server.serve_http_request(
        http::verb::get,
        "/api/v1/users/Rina Hidaka?gender=female&birth=1994/06/15",
        http_request().set_json({ { "message", "happy birthday" } }));
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.body_as_json<user_t>(),
             user_t {
                 .name = "Rina Hidaka",
                 .gender = "female",
                 .birth = "1994/06/15",
                 .message = "happy birthday",
             });
  }
  {
    auto res = server.serve_http_request(http::verb::get, "/api/v1/users",
                                         http_request());
    CHECK_EQ(res.status_code(), http::status::not_found);
  }
  {
    auto res = server.serve_http_request(
        http::verb::get, "/api/v1/users/Rina Hidaka", http_request());
    CHECK_EQ(res.status_code(), http::status::bad_request);
    CHECK_EQ(res.body_as_json(),
             json::value { { "error", "gender is not provided" } });
  }
  {
    auto res = server.serve_http_request(
        http::verb::get, "/api/v1/users/Rina Hidaka?gender=female",
        http_request());
    CHECK_EQ(res.status_code(), http::status::bad_request);
    CHECK_EQ(res.body_as_json(),
             json::value { { "error", "birth is not provided" } });
  }
  {
    auto res = server.serve_http_request(
        http::verb::get,
        "/api/v1/users/Rina Hidaka?gender=female&birth=1994/06/15",
        http_request());
    CHECK_EQ(res.status_code(), http::status::bad_request);
    CHECK_EQ(res.body_as_json(),
             json::value { { "error", "message is not provided" } });
  }
}

TEST_SUITE_END();
