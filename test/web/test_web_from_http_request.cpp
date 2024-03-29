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

TEST_SUITE_BEGIN("[fitoria.web.from_http_request]");

#if defined(FITORIA_HAS_BOOST_PFR)

struct date_t {
  std::string month;
  std::string day;
  std::string year;

  friend bool operator==(const date_t&, const date_t&) = default;
};

#endif

TEST_CASE("connection_info")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/">([](const connection_info& conn_info)
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
        "/", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

TEST_CASE("path_info")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/{user}">([](const path_info& path_info)
                                             -> net::awaitable<http_response> {
              CHECK_EQ(path_info.get("user"), "ramirisu");
              co_return http_response(http::status::ok);
            }))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/ramirisu", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

TEST_CASE("path<T = std::tuple<Ts...>>")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/{year}/{month}/{day}">(
                [](path<std::tuple<std::string, std::string, std::string>> path)
                    -> net::awaitable<http_response> {
                  auto [year, month, day] = std::move(path);
                  CHECK_EQ(year, "1994");
                  CHECK_EQ(month, "06");
                  CHECK_EQ(day, "15");
                  co_return http_response(http::status::ok);
                }))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/1994/06/15", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

TEST_CASE("path<T = std::tuple<Ts...>>, not match")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/{month}/{day}">(
                [](path<std::tuple<std::string, std::string, std::string>> path)
                    -> net::awaitable<http_response> {
                  auto [year, month, day] = std::move(path);
                  CHECK_EQ(year, "06");
                  CHECK_EQ(month, "06");
                  CHECK_EQ(day, "15");
                  co_return http_response(http::status::ok);
                }))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/06/15", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::internal_server_error);
  }());
}

#if defined(FITORIA_HAS_BOOST_PFR)

TEST_CASE("path<T = aggregate>")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/{year}/{month}/{day}">(
                [](path<date_t> path) -> net::awaitable<http_response> {
                  CHECK_EQ(
                      path,
                      date_t { .month = "06", .day = "15", .year = "1994" });
                  co_return http_response(http::status::ok);
                }))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/1994/06/15", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

TEST_CASE("path<T = aggregate>, not match")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/{month}/{day}">(
                [](path<date_t> path) -> net::awaitable<http_response> {
                  CHECK_EQ(
                      path,
                      date_t { .month = "06", .day = "15", .year = "1994" });
                  co_return http_response(http::status::ok);
                }))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/06/15", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::internal_server_error);
  }());
}

#endif

TEST_CASE("query_map")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/">(
                [](const query_map& query) -> net::awaitable<http_response> {
                  CHECK_EQ(query.get("year"), "1994");
                  CHECK_EQ(query.get("month"), "06");
                  CHECK_EQ(query.get("day"), "15");
                  co_return http_response(http::status::ok);
                }))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res
        = co_await server.async_serve_request("/",
                                              http_request(http::verb::get)
                                                  .set_query("year", "1994")
                                                  .set_query("month", "06")
                                                  .set_query("day", "15"));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

#if defined(FITORIA_HAS_BOOST_PFR)

TEST_CASE("query<T>")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/">(
                [](query<date_t> query) -> net::awaitable<http_response> {
                  CHECK_EQ(
                      query.get(),
                      date_t { .month = "06", .day = "15", .year = "1994" });
                  co_return http_response(http::status::ok);
                }))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res
        = co_await server.async_serve_request("/",
                                              http_request(http::verb::get)
                                                  .set_query("year", "1994")
                                                  .set_query("month", "06")
                                                  .set_query("day", "15"));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

#endif

TEST_CASE("http_fields")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/">(
                [](const http_fields& fields) -> net::awaitable<http_response> {
                  CHECK_EQ(fields.get(http::field::connection), "close");
                  co_return http_response(http::status::ok);
                }))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/",
        http_request(http::verb::get)
            .insert_field(http::field::connection, "close"));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

TEST_CASE("state<T>")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/">([](state<std::string> st)
                                       -> net::awaitable<http_response> {
                     CHECK_EQ(st, "shared state");
                     co_return http_response(http::status::ok);
                   }).state(std::string("shared state")))
            .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

TEST_CASE("std::string")
{
  auto server = http_server::builder()
                    .serve(route::post<"/">(
                        [](std::string text) -> net::awaitable<http_response> {
                          CHECK_EQ(text, "abc");
                          co_return http_response(http::status::ok);
                        }))
                    .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/", http_request(http::verb::post).set_body("abc"));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

TEST_CASE("std::vector<std::byte>")
{
  auto server
      = http_server::builder()
            .serve(route::post<"/">([](std::vector<std::byte> bytes)
                                        -> net::awaitable<http_response> {
              CHECK_EQ(bytes, str_to_vec<std::byte>("abc"));
              co_return http_response(http::status::ok);
            }))
            .build();
  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/", http_request(http::verb::post).set_body("abc"));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

TEST_CASE("std::vector<std::uint8_t>")
{
  auto server
      = http_server::builder()
            .serve(route::post<"/">([](std::vector<std::uint8_t> bytes)
                                        -> net::awaitable<http_response> {
              CHECK_EQ(bytes, str_to_vec<std::uint8_t>("abc"));
              co_return http_response(http::status::ok);
            }))
            .build();
  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/", http_request(http::verb::post).set_body("abc"));
    CHECK_EQ(res.status_code(), http::status::ok);
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

TEST_CASE("json<T>")
{
  auto server
      = http_server::builder()
            .serve(route::get<"/">(
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
          "/", http_request(http::verb::get).set_json(user));
      CHECK_EQ(res.status_code(), http::status::ok);
      CHECK_EQ(res.fields().get(http::field::content_type),
               http::fields::content_type::json());
      CHECK_EQ(co_await res.as_json<user_t>(), user);
    }
    {
      const auto json = boost::json::value { { "name", "Rina Hidaka" },
                                             { "birth", "1994/06/15" } };
      auto res = co_await server.async_serve_request(
          "/",
          http_request(http::verb::get).set_body(boost::json::serialize(json)));
      CHECK_EQ(res.status_code(), http::status::internal_server_error);
    }
    {
      const auto json = boost::json::value { { "name", "Rina Hidaka" } };
      auto res = co_await server.async_serve_request(
          "/", http_request(http::verb::get).set_json(json));
      CHECK_EQ(res.status_code(), http::status::internal_server_error);
    }
  }());
}

TEST_SUITE_END();
