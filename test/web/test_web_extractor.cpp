//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/utility.hpp>

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.extractor]");

TEST_CASE("connection_info extractor")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/get">([](const connection_info& conn_info)
                                          -> net::awaitable<http_response> {
              CHECK_EQ(conn_info.local_addr(),
                       net::ip::make_address("127.0.0.1"));
              CHECK_EQ(conn_info.local_port(), 0);
              CHECK_EQ(conn_info.remote_addr(),
                       net::ip::make_address("127.0.0.1"));
              CHECK_EQ(conn_info.remote_port(), 0);
              CHECK_EQ(conn_info.listen_addr(),
                       net::ip::make_address("127.0.0.1"));
              CHECK_EQ(conn_info.listen_port(), 0);
              co_return http_response(http::status::ok);
            }))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/get", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

TEST_CASE("path_info extractor")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/{user}">(
                [](path_info& path_info) -> net::awaitable<http_response> {
                  CHECK_EQ(path_info.get("user"), "gavin");
                  co_return http_response(http::status::ok);
                }))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/gavin", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

TEST_CASE("query_map extractor")
{
  auto server = http_server::builder()
                    .serve(route::get<"/get">(
                        [](query_map& query) -> net::awaitable<http_response> {
                          CHECK_EQ(query.get("user"), "gavin");
                          co_return http_response(http::status::ok);
                        }))
                    .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/get", http_request(http::verb::get).set_query("user", "gavin"));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

TEST_CASE("http_fields extractor")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/get">(
                [](http_fields& fields) -> net::awaitable<http_response> {
                  CHECK_EQ(fields.get(http::field::connection), "close");
                  co_return http_response(http::status::ok);
                }))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/get",
        http_request(http::verb::get)
            .insert_field(http::field::connection, "close"));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

TEST_CASE("state<T> extractor")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/get">([](state<std::string> s)
                                          -> net::awaitable<http_response> {
                     CHECK_EQ(s, "shared state");
                     co_return http_response(http::status::ok);
                   }).share_state(std::string("shared state")))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    {
      auto res = co_await server.async_serve_request(
          "/get", http_request(http::verb::get));
      CHECK_EQ(res.status_code(), http::status::ok);
    }
  }());
}

TEST_CASE("std::string extractor")
{
  auto server = http_server::builder()
                    .serve(route::post<"/post">(
                        [](std::string text) -> net::awaitable<http_response> {
                          CHECK_EQ(text, "abc");
                          co_return http_response(http::status::ok);
                        }))
                    .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    {
      auto res = co_await server.async_serve_request(
          "/post", http_request(http::verb::post).set_body("abc"));
      CHECK_EQ(res.status_code(), http::status::ok);
    }
  }());
}

TEST_CASE("std::vector<std::byte> extractor")
{
  auto server
      = http_server::builder()
            .serve(route::post<"/post">([](std::vector<std::byte> bytes)
                                            -> net::awaitable<http_response> {
              CHECK_EQ(bytes, to_bytes("abc"));
              co_return http_response(http::status::ok);
            }))
            .build();
  net::sync_wait([&]() -> net::awaitable<void> {
    {
      auto res = co_await server.async_serve_request(
          "/post", http_request(http::verb::post).set_body("abc"));
      CHECK_EQ(res.status_code(), http::status::ok);
    }
  }());
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

TEST_CASE("json<T> extractor")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/get">(
                [](json<user_t> user) -> net::awaitable<http_response> {
                  CHECK_EQ(user.name, "Rina Hidaka");
                  CHECK_EQ(user.birth, "1994/06/15");
                  co_return http_response(http::status::ok).set_json(user);
                }))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    {
      const auto user = user_t {
        .name = "Rina Hidaka",
        .birth = "1994/06/15",
      };
      auto res = co_await server.async_serve_request(
          "/get", http_request(http::verb::get).set_json(user));
      CHECK_EQ(res.status_code(), http::status::ok);
      CHECK_EQ(res.fields().get(http::field::content_type),
               http::fields::content_type::json());
      CHECK_EQ(co_await res.as_json<user_t>(), user);
    }
    {
      const auto json = boost::json::value { { "name", "Rina Hidaka" },
                                             { "birth", "1994/06/15" } };
      auto res = co_await server.async_serve_request(
          "/get",
          http_request(http::verb::get).set_body(boost::json::serialize(json)));
      CHECK_EQ(res.status_code(), http::status::bad_request);
    }
    {
      const auto json = boost::json::value { { "name", "Rina Hidaka" } };
      auto res = co_await server.async_serve_request(
          "/get", http_request(http::verb::get).set_json(json));
      CHECK_EQ(res.status_code(), http::status::bad_request);
    }
  }());
}

TEST_SUITE_END();
