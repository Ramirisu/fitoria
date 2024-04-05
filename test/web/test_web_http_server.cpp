//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/async_readable_chunk_stream.hpp>
#include <fitoria/test/cert.hpp>
#include <fitoria/test/http_server_utils.hpp>
#include <fitoria/test/utility.hpp>

#include <fitoria/client.hpp>
#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;
using fitoria::client::http_client;

TEST_SUITE_BEGIN("[fitoria.web.http_server]");

TEST_CASE("builder")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server_builder(ioc)
                    .set_max_listen_connections(2048)
                    .set_client_request_timeout(std::chrono::seconds(10))
                    .set_tls_handshake_timeout(std::chrono::seconds(5))
#if !FITORIA_NO_EXCEPTIONS
                    .set_exception_handler([](std::exception_ptr ptr) {
                      if (!ptr) {
                        return;
                      }
                      try {
                        std::rethrow_exception(ptr);
                      } catch (...) {
                      }
                    })
#endif
                    .serve(route::get<"/">(
                        [&](http_request&) -> net::awaitable<http_response> {
                          co_return http_response(http::status::ok);
                        }))
                    .build();
  CHECK(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  CHECK_EQ(server.max_listen_connections(), 2048);
  CHECK_EQ(server.client_request_timeout(), std::chrono::seconds(10));
  CHECK_EQ(server.tls_handshake_timeout(), std::chrono::seconds(5));

  net::co_spawn(
      ioc,
      [&]() -> net::awaitable<void> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::get)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .async_send();
        CHECK_EQ(res->status_code(), http::status::ok);
        CHECK_EQ(co_await res->as_string(), "");
      },
      net::use_future)
      .get();
}

TEST_CASE("socket reuse address")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server_builder(ioc).build();
  CHECK(server.bind(server_ip, port));
#if defined(FITORIA_TARGET_WINDOWS)
  CHECK(server.bind(server_ip, port));
#else
  CHECK(!server.bind(server_ip, port));
#endif
}

TEST_CASE("duplicate route")
{
  auto ioc = net::io_context();
  CHECK_THROWS_AS(
      auto server = http_server_builder(ioc).serve(
          scope()
              .serve(route::get<"/">(
                  [](http_request&) -> net::awaitable<http_response> {
                    co_return http_response(http::status::ok);
                  }))
              .serve(route::get<"/">(
                  [](http_request&) -> net::awaitable<http_response> {
                    co_return http_response(http::status::ok);
                  }))),
      std::system_error);
}

TEST_CASE("invalid target")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server_builder(ioc)
                    .serve(route::get<"/api/v1/users/{user}">(
                        [](http_request&) -> net::awaitable<http_response> {
                          co_return http_response(http::status::ok);
                        }))
                    .build();
  CHECK(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  const auto test_cases = std::vector {
    "/", "/a", "/api", "/api/", "/api/v1", "/api/v1/x", "/api/v1/users/x/y",
  };

  for (auto& test_case : test_cases) {
    net::co_spawn(
        ioc,
        [&]() -> net::awaitable<void> {
          auto res = co_await http_client()
                         .set_method(http::verb::get)
                         .set_url(to_local_url(
                             boost::urls::scheme::http, port, test_case))
                         .set_field(http::field::connection, "close")
                         .set_plaintext("text")
                         .async_send();
          CHECK_EQ(res->status_code(), http::status::not_found);
          CHECK_EQ(res->fields().get(http::field::content_type),
                   http::fields::content_type::plaintext());
          CHECK_EQ(co_await res->as_string(), "request path is not found");
        },
        net::use_future)
        .get();
  }
}

TEST_CASE("expect: 100-continue")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server_builder(ioc)
                    .serve(route::post<"/api/v1/post">(
                        [](std::string body) -> net::awaitable<http_response> {
                          CHECK_EQ(body, "text");
                          co_return http_response(http::status::ok);
                        }))
                    .build();
  CHECK(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> net::awaitable<void> {
        auto res = co_await http_client()
                       .set_method(http::verb::post)
                       .set_url(to_local_url(
                           boost::urls::scheme::http, port, "/api/v1/post"))
                       .set_field(http::field::expect, "100-continue")
                       .set_field(http::field::connection, "close")
                       .set_plaintext("text")
                       .async_send();
        CHECK_EQ(res->status_code(), http::status::ok);
        CHECK_EQ(co_await res->as_string(), "");
      },
      net::use_future)
      .get();
}

#if !FITORIA_NO_EXCEPTIONS

TEST_CASE("unhandled exception from handler")
{
  bool got_exception = false;
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server_builder(ioc)
                    .set_exception_handler([&](std::exception_ptr ptr) {
                      if (ptr) {
                        try {
                          std::rethrow_exception(ptr);
                        } catch (const std::exception&) {
                          got_exception = true;
                        }
                      }
                    })
                    .serve(route::get<"/">(
                        [](http_request&) -> net::awaitable<http_response> {
                          throw std::exception();
                          co_return http_response(http::status::ok);
                        }))
                    .build();
  CHECK(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> net::awaitable<void> {
        CHECK(!(co_await http_client()
                    .set_method(http::verb::get)
                    .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                    .set_field(http::field::connection, "close")
                    .set_plaintext("text")
                    .async_send()));
      },
      net::use_future)
      .get();

  // wait for exception thrown
  std::this_thread::sleep_for(std::chrono::seconds(1));
  CHECK(got_exception);
}

#endif

TEST_CASE("generic request")
{
  const auto text = std::string_view("happy birthday");

  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/api/v1/users/{user}/filmography/years/{year}">(
                [=](http_request& req,
                    const connection_info& connection,
                    const path_info& path,
                    const query_map& query,
                    const http_fields& fields,
                    std::string body) -> net::awaitable<http_response> {
                  auto test_connection = [=](auto& conn) {
                    CHECK_EQ(conn.local().address(),
                             net::ip::make_address(server_ip));
                    CHECK_EQ(conn.remote().address(),
                             net::ip::make_address(server_ip));
                  };
                  test_connection(req.connection());
                  test_connection(connection);

                  CHECK_EQ(req.method(), http::verb::get);

                  auto test_path = [](auto& path) {
                    CHECK_EQ(path.match_pattern(),
                             "/api/v1/users/{user}/filmography/years/{year}");
                    CHECK_EQ(path.match_path(),
                             "/api/v1/users/RinaHidaka/filmography/years/2022");

                    CHECK_EQ(path.at("user"), "RinaHidaka");
                    CHECK_EQ(path.at("year"), "2022");
                  };
                  test_path(req.path());
                  test_path(path);

                  auto test_query = [](auto& query) {
                    CHECK_EQ(query.size(), 2);
                    CHECK_EQ(query.at("name"), "Rina Hidaka");
                    CHECK_EQ(query.at("birth"), "1994/06/15");
                  };
                  test_query(req.query());
                  test_query(static_cast<const http_request&>(req).query());
                  test_query(query);

                  auto test_fields = [](auto& fields) {
                    CHECK_EQ(fields.get(http::field::content_type),
                             http::fields::content_type::plaintext());
                  };

                  test_fields(req.fields());
                  test_fields(static_cast<const http_request&>(req).fields());
                  test_fields(fields);

                  CHECK(range_in_set(
                      req.fields().equal_range(http::field::user_agent),
                      [](auto&& p) { return p->value(); },
                      std::set<std::string_view> { BOOST_BEAST_VERSION_STRING,
                                                   "fitoria" }));

                  CHECK_EQ(body, "happy birthday");
                  auto buffer = std::array<std::byte, 4096>();
                  CHECK(!(co_await req.body().async_read_some(
                      net::buffer(buffer))));
                  CHECK(!(
                      co_await async_read_until_eof<std::string>(req.body())));

                  co_return http_response(http::status::ok)
                      .insert_field(http::field::user_agent,
                                    BOOST_BEAST_VERSION_STRING)
                      .insert_field(http::field::user_agent, "fitoria");
                }))
            .build();
  CHECK(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> net::awaitable<void> {
        auto res = co_await http_client()
                       .set_method(http::verb::get)
                       .set_url(to_local_url(
                           boost::urls::scheme::http,
                           port,
                           "/api/v1/users/RinaHidaka/filmography/years/2022"))
                       .set_query("name", "Rina Hidaka")
                       .set_query("birth", "1994/06/15")
                       .set_field(http::field::connection, "close")
                       .insert_field(http::field::user_agent,
                                     BOOST_BEAST_VERSION_STRING)
                       .insert_field(http::field::user_agent, "fitoria")
                       .set_plaintext(text)
                       .async_send();
        CHECK_EQ(res->status_code(), http::status::ok);
        CHECK(range_in_set(
            res->fields().equal_range(http::field::user_agent),
            [](auto&& p) { return p->value(); },
            std::set<std::string_view> { BOOST_BEAST_VERSION_STRING,
                                         "fitoria" }));
        CHECK_EQ(co_await res->as_string(), "");
      },
      net::use_future)
      .get();
}

TEST_CASE("request to route accepting wildcard")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/api/v1/#wildcard">(
                [=](const path_info& path_info)
                    -> net::awaitable<http_response> {
                  CHECK_EQ(path_info.match_pattern(), "/api/v1/#wildcard");
                  CHECK_EQ(path_info.match_path(), "/api/v1/any/path");
                  CHECK_EQ(path_info.get("wildcard"), "any/path");
                  co_return http_response(http::status::ok);
                }))
            .build();
  CHECK(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> net::awaitable<void> {
        auto res = co_await http_client()
                       .set_method(http::verb::get)
                       .set_url(to_local_url(
                           boost::urls::scheme::http, port, "/api/v1/any/path"))
                       .set_field(http::field::connection, "close")
                       .async_send();
        CHECK_EQ(res->status_code(), http::status::ok);
        CHECK_EQ(co_await res->as_string(), "");
      },
      net::use_future)
      .get();
}

TEST_CASE("request with null body")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/">(
                [](http_request& req) -> net::awaitable<http_response> {
                  CHECK_EQ(req.fields().get(http::field::connection), "close");
                  CHECK(!req.fields().get(http::field::content_length));
                  CHECK_EQ(req.body().size_hint(), std::size_t(0));
                  CHECK_EQ(
                      (co_await async_read_until_eof<std::string>(req.body()))
                          .error(),
                      make_error_code(net::error::eof));
                  co_return http_response(http::status::ok);
                }))
            .build();
  CHECK(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> net::awaitable<void> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::get)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_field(http::field::connection, "close")
                  .async_send();
        CHECK_EQ(res->status_code(), http::status::ok);
      },
      net::use_future)
      .get();
}

TEST_CASE("request with empty body")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::post<"/">([](const http_request& req, std::string str)
                                        -> net::awaitable<http_response> {
              CHECK_EQ(req.fields().get(http::field::connection), "close");
              CHECK_EQ(req.fields().get(http::field::content_length), "0");
              CHECK_EQ(req.body().size_hint(), std::size_t(0));
              CHECK_EQ(str, "");
              co_return http_response(http::status::ok);
            }))
            .build();
  CHECK(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> net::awaitable<void> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::post)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_field(http::field::connection, "close")
                  .set_plaintext("")
                  .async_send();
        CHECK_EQ(res->status_code(), http::status::ok);
      },
      net::use_future)
      .get();
}

TEST_CASE("request with stream (chunked transfer-encoding)")
{
  const auto text = std::string_view("abcdefghijklmnopqrstuvwxyz");

  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::post<"/">(
                [text](const http_request& req,
                       std::string data) -> net::awaitable<http_response> {
                  CHECK_EQ(req.fields().get(http::field::content_type),
                           http::fields::content_type::plaintext());
                  CHECK(!req.fields().get(http::field::content_length));
                  CHECK(!req.body().size_hint());
                  CHECK_EQ(data, text);
                  co_return http_response(http::status::ok);
                }))
            .build();
  CHECK(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> net::awaitable<void> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::post)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_field(http::field::connection, "close")
                  .set_field(http::field::content_type,
                             http::fields::content_type::plaintext())
                  .set_stream(async_readable_chunk_stream<5>(text))
                  .async_send();
        CHECK_EQ(res->status_code(), http::status::ok);
        CHECK_EQ(co_await res->as_string(), "");
      },
      net::use_future)
      .get();
}

TEST_CASE("response status only")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server_builder(ioc)
                    .serve(route::get<"/">(
                        [](http_request&) -> net::awaitable<http_response> {
                          co_return http_response(http::status::no_content);
                        }))
                    .build();
  CHECK(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> net::awaitable<void> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::get)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_field(http::field::connection, "close")
                  .async_send();
        CHECK_EQ(res->status_code(), http::status::no_content);
        CHECK_EQ(res->fields().get(http::field::connection), "close");
        CHECK(!res->fields().get(http::field::content_length));
        CHECK_EQ(res->body().size_hint(), std::size_t(0));
        CHECK(!(co_await res->as_string()));
      },
      net::use_future)
      .get();
}

TEST_CASE("response with plain text")
{
  const auto text = std::string_view("plain text");

  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/">(
                [text](http_request&) -> net::awaitable<http_response> {
                  co_return http_response(http::status::ok)
                      .set_field(http::field::content_type,
                                 http::fields::content_type::plaintext())
                      .set_body(text);
                }))
            .build();
  CHECK(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> net::awaitable<void> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::get)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_field(http::field::connection, "close")
                  .async_send();
        CHECK_EQ(res->status_code(), http::status::ok);
        CHECK_EQ(res->fields().get(http::field::connection), "close");
        CHECK_EQ(res->fields().get(http::field::content_type),
                 http::fields::content_type::plaintext());
        CHECK_EQ(res->fields().get(http::field::content_length),
                 std::to_string(text.size()));
        CHECK_EQ(res->body().size_hint(), text.size());
        CHECK_EQ(co_await res->as_string(), text);
      },
      net::use_future)
      .get();
}

TEST_CASE("response with with stream (chunked transfer-encoding)")
{
  const auto text = std::string_view("abcdefghijklmnopqrstuvwxyz");

  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/">(
                [text](http_request&) -> net::awaitable<http_response> {
                  co_return http_response(http::status::ok)
                      .set_field(http::field::content_type,
                                 http::fields::content_type::plaintext())
                      .set_stream(async_readable_chunk_stream<5>(text));
                }))
            .build();
  CHECK(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> net::awaitable<void> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::get)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_field(http::field::connection, "close")
                  .async_send();
        CHECK_EQ(res->status_code(), http::status::ok);
        CHECK_EQ(res->fields().get(http::field::content_type),
                 http::fields::content_type::plaintext());
        CHECK(!res->fields().get(http::field::content_length));
        CHECK(!res->body().size_hint());
        CHECK_EQ(co_await res->as_string(), text);
      },
      net::use_future)
      .get();
}

TEST_SUITE_END();
