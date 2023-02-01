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

json::result_for<user_t, json::value>::type
tag_invoke(const json::try_value_to_tag<user_t>&, const json::value& jv)
{
  user_t user;

  if (!jv.is_object()) {
    return make_error_code(json::error::incomplete);
  }

  const auto& obj = jv.get_object();

  auto* name = obj.if_contains("name");
  auto* gender = obj.if_contains("gender");
  auto* birth = obj.if_contains("birth");
  auto* message = obj.if_contains("message");
  if (name && gender && birth && message && name->is_string()
      && gender->is_string() && birth->is_string() && message->is_string()) {
    return user_t {
      .name = std::string(name->get_string()),
      .gender = std::string(gender->get_string()),
      .birth = std::string(birth->get_string()),
      .message = std::string(message->get_string()),
    };
  }

  return make_error_code(json::error::incomplete);
}

}

TEST_CASE("unittest")
{
  auto server
      = http_server::builder()
            .route(route(
                http::verb::get, "/api/v1/users/{user}",
                [](http_request& req) -> net::awaitable<http_response> {
                  user_t user;
                  user.name = req.route_params().get("user").value();
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
                  if (auto ct = req.fields().get(http::field::content_type);
                      ct != http::fields::content_type::json()) {
                    co_return http_response(http::status::bad_request)
                        .set_json({ { "error",
                                      fmt::format("expected Content-Type: "
                                                  "application/json, got {}",
                                                  ct) } });
                  }
                  if (auto msg
                      = value_to_optional(as_json(req.body()))
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
        "/api/v1/users/Rina Hidaka",
        http_request()
            .set_method(http::verb::get)
            .set_query("gender", "female")
            .set_query("birth", "1994/06/15")
            .set_json({ { "message", "happy birthday" } }));
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.fields().get(http::field::content_type),
             http::fields::content_type::json());
    CHECK_EQ(as_json<user_t>(res.body()),
             user_t {
                 .name = "Rina Hidaka",
                 .gender = "female",
                 .birth = "1994/06/15",
                 .message = "happy birthday",
             });
  }
  {
    auto res = server.serve_http_request(
        "/api/v1/users", http_request().set_method(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::not_found);
  }
  {
    auto res
        = server.serve_http_request("/api/v1/users/Rina Hidaka",
                                    http_request().set_method(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::bad_request);
    CHECK_EQ(res.fields().get(http::field::content_type),
             http::fields::content_type::json());
    CHECK_EQ(as_json(res.body()),
             json::value { { "error", "gender is not provided" } });
  }
  {
    auto res = server.serve_http_request("/api/v1/users/Rina Hidaka",
                                         http_request()
                                             .set_method(http::verb::get)
                                             .set_query("gender", "female"));
    CHECK_EQ(res.status_code(), http::status::bad_request);
    CHECK_EQ(res.fields().get(http::field::content_type),
             http::fields::content_type::json());
    CHECK_EQ(as_json(res.body()),
             json::value { { "error", "birth is not provided" } });
  }
  {
    auto res = server.serve_http_request("/api/v1/users/Rina Hidaka",
                                         http_request()
                                             .set_method(http::verb::get)
                                             .set_query("gender", "female")
                                             .set_query("birth", "1994/06/15"));
    CHECK_EQ(res.status_code(), http::status::bad_request);
    CHECK_EQ(res.fields().get(http::field::content_type),
             http::fields::content_type::json());
    CHECK_EQ(
        as_json(res.body()),
        json::value {
            { "error",
              "expected Content-Type: application/json, got {nullopt}" } });
  }
  {
    auto res = server.serve_http_request(
        "/api/v1/users/Rina Hidaka",
        http_request()
            .set_method(http::verb::get)
            .set_query("gender", "female")
            .set_query("birth", "1994/06/15")
            .set_field(http::field::content_type,
                       http::fields::content_type::json()));
    CHECK_EQ(res.status_code(), http::status::bad_request);
    CHECK_EQ(res.fields().get(http::field::content_type),
             http::fields::content_type::json());
    CHECK_EQ(as_json(res.body()),
             json::value { { "error", "message is not provided" } });
  }
}

TEST_SUITE_END();
