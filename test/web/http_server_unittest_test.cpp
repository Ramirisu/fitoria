//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("web.http_server.unittest");

TEST_CASE("connection_info")
{
  auto server = http_server::builder()
                    .route(route::GET<"/get">(
                        [](http_request& req) -> lazy<http_response> {
                          CHECK_EQ(req.conn_info().local_addr(),
                                   net::ip::make_address("127.0.0.1"));
                          CHECK_EQ(req.conn_info().local_port(), 0);
                          CHECK_EQ(req.conn_info().remote_addr(),
                                   net::ip::make_address("127.0.0.1"));
                          CHECK_EQ(req.conn_info().remote_port(), 0);
                          CHECK_EQ(req.conn_info().listen_addr(),
                                   net::ip::make_address("127.0.0.1"));
                          CHECK_EQ(req.conn_info().listen_port(), 0);
                          co_return http_response(http::status::ok);
                        }))
                    .build();
  {
    auto res
        = server.serve_http_request("/get", mock_http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
  }
}

TEST_CASE("state")
{
  struct shared_resource {
    std::string_view value;
  };

  auto server
      = http_server::builder()
            .route(
                scope<"">()
                    .state(shared_resource { "global" })
                    .sub_scope(
                        scope<"/api/v1">()
                            .GET<"/global">(
                                [](http_request& req) -> lazy<http_response> {
                                  co_return http_response(http::status::ok)
                                      .set_body(std::string(
                                          req.state<const shared_resource&>()
                                              ->value));
                                })
                            .state(shared_resource { "scope" })
                            .GET<"/scope">(
                                [](http_request& req) -> lazy<http_response> {
                                  co_return http_response(http::status::ok)
                                      .set_body(std::string(
                                          req.state<const shared_resource&>()
                                              ->value));
                                })
                            .route(route::GET<
                                       "/route">([](http_request& req)
                                                     -> lazy<http_response> {
                                     co_return http_response(http::status::ok)
                                         .set_body(std::string(
                                             req.state<const shared_resource&>()
                                                 ->value));
                                   }).state(shared_resource { "route" }))))
            .build();
  {
    auto res = server.serve_http_request("/api/v1/global",
                                         mock_http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.body(), "global");
  }
  {
    auto res = server.serve_http_request("/api/v1/scope",
                                         mock_http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.body(), "scope");
  }
  {
    auto res = server.serve_http_request("/api/v1/route",
                                         mock_http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.body(), "route");
  }
}

TEST_CASE("string extractor")
{
  auto server = http_server::builder()
                    .route(route::POST<"/post">(
                        [](std::string text) -> lazy<http_response> {
                          CHECK_EQ(text, "abc");
                          co_return http_response(http::status::ok);
                        }))
                    .build();
  {
    auto res = server.serve_http_request(
        "/post", mock_http_request(http::verb::post).set_body("abc"));
    CHECK_EQ(res.status_code(), http::status::ok);
  }
}

TEST_CASE("bytes extractor")
{
  auto server
      = http_server::builder()
            .route(route::POST<"/post">(
                [](std::vector<std::byte> bytes) -> lazy<http_response> {
                  CHECK_EQ(bytes, to_bytes("abc"));
                  co_return http_response(http::status::ok);
                }))
            .build();
  {
    auto res = server.serve_http_request(
        "/post", mock_http_request(http::verb::post).set_body("abc"));
    CHECK_EQ(res.status_code(), http::status::ok);
  }
}

namespace {

struct user_t {
  std::string name;
  std::string birth;

  friend bool operator==(const user_t&, const user_t&) = default;
};

void tag_invoke(const boost::json::value_from_tag&,
                boost::json::value& jv,
                const user_t& user)
{
  jv = {
    { "name", user.name },
    { "birth", user.birth },
  };
}

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
  auto* birth = obj.if_contains("birth");
  if (name && birth && name->is_string() && birth->is_string()) {
    return user_t {
      .name = std::string(name->get_string()),
      .birth = std::string(birth->get_string()),
    };
  }

  return make_error_code(boost::json::error::incomplete);
}

}

TEST_CASE("json")
{
  auto server
      = http_server::builder()
            .route(route::GET<"/get">(
                [](json<user_t> user) -> lazy<http_response> {
                  CHECK_EQ(user.name, "Rina Hidaka");
                  CHECK_EQ(user.birth, "1994/06/15");
                  co_return http_response(http::status::ok).set_json(user);
                }))
            .build();
  {
    auto res = server.serve_http_request(
        "/get",
        mock_http_request(http::verb::get)
            .set_json(
                { { "name", "Rina Hidaka" }, { "birth", "1994/06/15" } }));
    CHECK_EQ(res.status_code(), http::status::ok);
    CHECK_EQ(res.fields().get(http::field::content_type),
             http::fields::content_type::json());
    CHECK_EQ(as_json<user_t>(res.body()),
             user_t {
                 .name = "Rina Hidaka",
                 .birth = "1994/06/15",
             });
  }
  {
    auto res = server.serve_http_request(
        "/get",
        mock_http_request(http::verb::get)
            .set_body(boost::json::serialize(boost::json::value {
                { "name", "Rina Hidaka" }, { "birth", "1994/06/15" } })));
    CHECK_EQ(res.status_code(), http::status::bad_request);
  }
  {
    auto res = server.serve_http_request(
        "/get",
        mock_http_request(http::verb::get)
            .set_json({ { "name", "Rina Hidaka" } }));
    CHECK_EQ(res.status_code(), http::status::bad_request);
  }
}

TEST_SUITE_END();
