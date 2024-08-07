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

TEST_SUITE_BEGIN("[fitoria.web.from_request]");

struct date_t {
  std::string month;
  std::string day;
  int year;

  friend bool operator==(const date_t&, const date_t&) = default;
};

TEST_CASE("connect_info")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">(
                [](const connect_info& connection) -> awaitable<response> {
                  CHECK_EQ(connection.local(), "localhost");
                  CHECK_EQ(connection.remote(), "localhost");
                  co_return response::ok().build();
                }))
            .build();

  server.serve_request("/",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });

  ioc.run();
}

TEST_CASE("path_info")
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/{user}">(
                        [](const path_info& path_info) -> awaitable<response> {
                          CHECK_EQ(path_info.get("user"), "fitoria");
                          co_return response::ok().build();
                        }))
                    .build();

  server.serve_request("/fitoria",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });

  ioc.run();
}

TEST_CASE("path_of<T = std::tuple<Ts...>>")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/{year}/{month}/{day}">(
                [](path_of<std::tuple<int, std::string, std::uint8_t>> path)
                    -> awaitable<response> {
                  auto [year, month, day] = std::move(path);
                  CHECK_EQ(year, 1994);
                  CHECK_EQ(month, "June");
                  CHECK_EQ(day, 15);
                  co_return response::ok().build();
                }))
            .serve(route::get<"/{month}/{day}">(
                [](path_of<std::tuple<int, std::string, std::uint8_t>> path)
                    -> awaitable<response> {
                  auto [year, month, day] = std::move(path);
                  CHECK_EQ(year, 1994);
                  CHECK_EQ(month, "June");
                  CHECK_EQ(day, 15);
                  co_return response::ok().build();
                }))
            .build();

  server.serve_request("/1994/June/15",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });
  server.serve_request("/2147483648/June/15",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         co_return;
                       });
  server.serve_request("/06/15",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         co_return;
                       });

  ioc.run();
}

TEST_CASE("path_of<T = aggregate>")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/{year}/{month}/{day}">(
                [](path_of<date_t> path) -> awaitable<response> {
                  CHECK_EQ(path,
                           date_t { .month = "06", .day = "15", .year = 1994 });
                  co_return response::ok().build();
                }))
            .serve(route::get<"/{month}/{day}">(
                [](path_of<date_t> path) -> awaitable<response> {
                  CHECK_EQ(path,
                           date_t { .month = "06", .day = "15", .year = 1994 });
                  co_return response::ok().build();
                }))
            .build();

  server.serve_request("/1994/06/15",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });
  server.serve_request("/2147483648/June/15",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         co_return;
                       });
  server.serve_request("/06/15",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(),
                                  http::status::internal_server_error);
                         co_return;
                       });

  ioc.run();
}

TEST_CASE("query_map")
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">(
                        [](const query_map& query) -> awaitable<response> {
                          CHECK_EQ(query.get("year"), "1994");
                          CHECK_EQ(query.get("month"), "06");
                          CHECK_EQ(query.get("day"), "15");
                          co_return response::ok().build();
                        }))
                    .build();

  server.serve_request("/",
                       test_request::get()
                           .set_query("year", "1994")
                           .set_query("month", "06")
                           .set_query("day", "15")
                           .build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });

  ioc.run();
}

TEST_CASE("query_of<T>")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">(
                [](query_of<date_t> query) -> awaitable<response> {
                  CHECK_EQ(query,
                           date_t { .month = "06", .day = "15", .year = 1994 });
                  co_return response::ok().build();
                }))
            .build();

  server.serve_request("/",
                       test_request::get()
                           .set_query("year", "1994")
                           .set_query("month", "06")
                           .set_query("day", "15")
                           .build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });
  server.serve_request("/",
                       test_request::get()
                           .set_query("year", "2147483648")
                           .set_query("month", "06")
                           .set_query("day", "15")
                           .build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         co_return;
                       });

  server.serve_request("/",
                       test_request::get()
                           .set_query("y", "1994")
                           .set_query("month", "06")
                           .set_query("day", "15")
                           .build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         co_return;
                       });
  server.serve_request("/",
                       test_request::get()
                           .set_query("month", "06")
                           .set_query("day", "15")
                           .build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         co_return;
                       });

  ioc.run();
}

TEST_CASE("form_of<T>")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">(
                [](form_of<date_t> query) -> awaitable<response> {
                  CHECK_EQ(query,
                           date_t { .month = "06", .day = "15", .year = 1994 });
                  co_return response::ok().build();
                }))
            .build();

  server.serve_request("/",
                       test_request::get()
                           .set_header(http::field::content_type,
                                       mime::application_www_form_urlencoded())
                           .set_body("year=1994&month=06&day=15"),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });
  server.serve_request(
      "/",
      test_request::get()
          .set_header(http::field::content_type,
                      "application/x-www-form-urlencoded; charset=utf-8")
          .set_body("year=1994&month=06&day=15"),
      [](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::ok);
        co_return;
      });
  server.serve_request(
      "/",
      test_request::get().set_body("year=1994&month=06&day=15"),
      [](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::bad_request);
        co_return;
      });
  server.serve_request(
      "/",
      test_request::get()
          .set_header(http::field::content_type, mime::text_plain())
          .set_body("year=1994&month=06&day=15"),
      [](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::bad_request);
        co_return;
      });
  server.serve_request(
      "/",
      test_request::get()
          .set_header(http::field::content_type, "application=form")
          .set_body("year=1994&month=06&day=15"),
      [](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::bad_request);
        co_return;
      });
  server.serve_request("/",
                       test_request::get()
                           .set_header(http::field::content_type,
                                       mime::application_www_form_urlencoded())
                           .set_body("&&year=1994&&&"),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         co_return;
                       });
  server.serve_request("/",
                       test_request::get()
                           .set_header(http::field::content_type,
                                       mime::application_www_form_urlencoded())
                           .set_body("month=06&day=15"),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         co_return;
                       });
  server.serve_request("/",
                       test_request::get()
                           .set_header(http::field::content_type,
                                       mime::application_www_form_urlencoded())
                           .set_body("year=2147483648&month=06&day=15"),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         co_return;
                       });
  server.serve_request("/",
                       test_request::get()
                           .set_header(http::field::content_type,
                                       mime::application_www_form_urlencoded())
                           .set_body("y=1994&month=06&day=15"),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         co_return;
                       });

  ioc.run();
}

TEST_CASE("http::header_map")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">(
                [](const http::header_map& headers) -> awaitable<response> {
                  CHECK_EQ(headers.get(http::field::connection), "close");
                  co_return response::ok().build();
                }))
            .build();

  server.serve_request("/",
                       test_request::get()
                           .insert_header(http::field::connection, "close")
                           .build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });

  ioc.run();
}

TEST_CASE("state_of<T>")
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">([](state_of<std::string> st)
                                               -> awaitable<response> {
                             CHECK_EQ(st, "shared state");
                             co_return response::ok().build();
                           }).use_state(std::string("shared state")))
                    .serve(route::get<"/no_state">(
                        [](state_of<std::string> st) -> awaitable<response> {
                          CHECK_EQ(st, "shared state");
                          co_return response::ok().build();
                        }))
                    .build();

  server.serve_request("/",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });
  server.serve_request("/no_state",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(),
                                  http::status::internal_server_error);
                         co_return;
                       });

  ioc.run();
}

TEST_CASE("std::string")
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::post<"/">(
                        [](std::string text) -> awaitable<response> {
                          CHECK_EQ(text, "abc");
                          co_return response::ok().build();
                        }))
                    .build();

  server.serve_request("/",
                       test_request::post().set_body("abc"),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });

  ioc.run();
}

TEST_CASE("bytes")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::post<"/">([](bytes bs) -> awaitable<response> {
              CHECK_EQ(bs, str_to_vec<std::byte>("abc"));
              co_return response::ok().build();
            }))
            .build();

  server.serve_request("/",
                       test_request::post().set_body("abc"),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });

  ioc.run();
}

TEST_CASE("std::vector<std::uint8_t>")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::post<"/">(
                [](std::vector<std::uint8_t> bytes) -> awaitable<response> {
                  CHECK_EQ(bytes, str_to_vec<std::uint8_t>("abc"));
                  co_return response::ok().build();
                }))
            .build();

  server.serve_request("/",
                       test_request::post().set_body("abc"),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });

  ioc.run();
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

TEST_CASE("json_of<T>")
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::post<"/">(
                        [](json_of<user_t> user) -> awaitable<response> {
                          CHECK_EQ(user.name, "Rina Hidaka");
                          CHECK_EQ(user.birth, "1994/06/15");
                          co_return response::ok().set_json(user);
                        }))
                    .build();

  const auto user = user_t {
    .name = "Rina Hidaka",
    .birth = "1994/06/15",
  };
  const auto json = boost::json::value { { "name", "Rina Hidaka" },
                                         { "birth", "1994/06/15" } };
  server.serve_request("/",
                       test_request::post().set_json(user),
                       [=](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         CHECK_EQ(res.headers().get(http::field::content_type),
                                  mime::application_json());
                         CHECK_EQ(co_await res.template as_json<user_t>(),
                                  user);
                       });
  server.serve_request("/",
                       test_request::post()
                           .set_header(http::field::content_type,
                                       "application/json; charset=utf-8")
                           .set_body(boost::json::serialize(json)),
                       [=](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         CHECK_EQ(res.headers().get(http::field::content_type),
                                  mime::application_json());
                         CHECK_EQ(co_await res.template as_json<user_t>(),
                                  user);
                       });
  server.serve_request("/",
                       test_request::post().set_json(json),
                       [&json](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         CHECK_EQ(res.headers().get(http::field::content_type),
                                  mime::application_json());
                         CHECK_EQ(co_await res.template as_json<user_t>(),
                                  *boost::json::try_value_to<user_t>(json));
                       });
  server.serve_request(
      "/",
      test_request::post().set_body(boost::json::serialize(json)),
      [](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::bad_request);
        co_return;
      });
  server.serve_request(
      "/",
      test_request::post()
          .set_header(http::field::content_type, "application=json")
          .set_body(boost::json::serialize(json)),
      [](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::bad_request);
        co_return;
      });
  server.serve_request(
      "/",
      test_request::post()
          .set_header(http::field::content_type, mime::text_plain())
          .set_body(boost::json::serialize(json)),
      [](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::bad_request);
        co_return;
      });

  const auto incorrect = boost::json::value { { "name", "Rina Hidaka" } };
  server.serve_request("/",
                       test_request::post().set_json(incorrect),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         co_return;
                       });
  server.serve_request("/",
                       test_request::post().set_json("name:"),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         co_return;
                       });

  ioc.run();
}

TEST_SUITE_END();
