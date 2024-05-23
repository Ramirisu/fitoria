//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/async_readable_chunk_stream.hpp>
#include <fitoria/test/http_server_utils.hpp>
#include <fitoria/test/utility.hpp>

#include <fitoria/client.hpp>
#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;
using fitoria::client::http_client;

TEST_SUITE_BEGIN("[fitoria.web.http_server.request]");

TEST_CASE("generic request")
{
  const auto text = std::string_view("happy birthday");

  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/api/v1/users/{user}/filmography/years/{year}">(
                [=](request& req,
                    const connect_info& connection,
                    const path_info& path,
                    const http::version& ver,
                    const http::header& header,
                    const query_map& query,
                    std::string body) -> awaitable<response> {
                  auto test_connection = [=](auto& conn) {
                    REQUIRE_EQ(conn.local().address(),
                               net::ip::make_address(server_ip));
                    REQUIRE_EQ(conn.remote().address(),
                               net::ip::make_address(server_ip));
                  };
                  test_connection(req.connection());
                  test_connection(connection);

                  REQUIRE_EQ(req.method(), http::verb::get);

                  auto test_path = [](auto& path) {
                    REQUIRE_EQ(path.match_pattern(),
                               "/api/v1/users/{user}/filmography/years/{year}");
                    REQUIRE_EQ(
                        path.match_path(),
                        "/api/v1/users/RinaHidaka/filmography/years/2022");

                    REQUIRE_EQ(path.at("user"), "RinaHidaka");
                    REQUIRE_EQ(path.at("year"), "2022");
                  };
                  test_path(req.path());
                  test_path(path);

                  REQUIRE_EQ(ver, http::version::v1_1);

                  auto test_query = [](auto& query) {
                    REQUIRE_EQ(query.size(), 2);
                    REQUIRE_EQ(query.at("name"), "Rina Hidaka");
                    REQUIRE_EQ(query.at("birth"), "1994/06/15");
                  };
                  test_query(req.query());
                  test_query(static_cast<const request&>(req).query());
                  test_query(query);

                  auto test_header = [](auto& header) {
                    REQUIRE_EQ(header.get(http::field::content_type),
                               http::fields::content_type::plaintext());
                  };

                  test_header(req.header());
                  test_header(static_cast<const request&>(req).header());
                  test_header(header);

                  REQUIRE(range_in_set(
                      req.header().equal_range(http::field::user_agent),
                      [](auto&& p) { return p->value(); },
                      std::set<std::string_view> { BOOST_BEAST_VERSION_STRING,
                                                   "fitoria" }));

                  REQUIRE_EQ(body, "happy birthday");
                  auto buffer = std::array<std::byte, 4096>();
                  REQUIRE(!(co_await req.body().async_read_some(
                      net::buffer(buffer))));
                  REQUIRE(!(
                      co_await async_read_until_eof<std::string>(req.body())));

                  co_return response::ok()
                      .insert_header(http::field::user_agent,
                                     BOOST_BEAST_VERSION_STRING)
                      .insert_header(http::field::user_agent, "fitoria")
                      .build();
                }))
            .build();
  REQUIRE(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        auto res = co_await http_client()
                       .set_method(http::verb::get)
                       .set_url(to_local_url(
                           boost::urls::scheme::http,
                           port,
                           "/api/v1/users/RinaHidaka/filmography/years/2022"))
                       .set_query("name", "Rina Hidaka")
                       .set_query("birth", "1994/06/15")
                       .set_header(http::field::connection, "close")
                       .insert_header(http::field::user_agent,
                                      BOOST_BEAST_VERSION_STRING)
                       .insert_header(http::field::user_agent, "fitoria")
                       .set_plaintext(text)
                       .async_send();
        REQUIRE_EQ(res->status_code(), http::status::ok);
        REQUIRE(range_in_set(
            res->header().equal_range(http::field::user_agent),
            [](auto&& p) { return p->value(); },
            std::set<std::string_view> { BOOST_BEAST_VERSION_STRING,
                                         "fitoria" }));
        REQUIRE_EQ(co_await res->as_string(), "");
      },
      net::use_future)
      .get();
}

TEST_CASE("request to route accepting wildcard")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/api/v1/#wildcard">(
                [=](const path_info& path_info) -> awaitable<response> {
                  REQUIRE_EQ(path_info.match_pattern(), "/api/v1/#wildcard");
                  REQUIRE_EQ(path_info.match_path(), "/api/v1/any/path");
                  REQUIRE_EQ(path_info.get("wildcard"), "any/path");
                  co_return response::ok().build();
                }))
            .build();
  REQUIRE(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        auto res = co_await http_client()
                       .set_method(http::verb::get)
                       .set_url(to_local_url(
                           boost::urls::scheme::http, port, "/api/v1/any/path"))
                       .set_header(http::field::connection, "close")
                       .async_send();
        REQUIRE_EQ(res->status_code(), http::status::ok);
        REQUIRE_EQ(co_await res->as_string(), "");
      },
      net::use_future)
      .get();
}

TEST_CASE("request with null body")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">([](request& req) -> awaitable<response> {
              REQUIRE_EQ(req.header().get(http::field::connection), "close");
              REQUIRE(!req.header().get(http::field::content_length));
              REQUIRE(
                  !(co_await async_read_until_eof<std::string>(req.body())));
              co_return response::ok().build();
            }))
            .build();
  REQUIRE(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::get)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_header(http::field::connection, "close")
                  .async_send();
        REQUIRE_EQ(res->status_code(), http::status::ok);
      },
      net::use_future)
      .get();
}

TEST_CASE("request with empty body")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::post<"/">([](const request& req,
                                       std::string str) -> awaitable<response> {
              REQUIRE_EQ(req.header().get(http::field::connection), "close");
              REQUIRE_EQ(req.header().get(http::field::content_length), "0");
              REQUIRE_EQ(str, "");
              co_return response::ok().build();
            }))
            .build();
  REQUIRE(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::post)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_header(http::field::connection, "close")
                  .set_plaintext("")
                  .async_send();
        REQUIRE_EQ(res->status_code(), http::status::ok);
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
      = http_server::builder(ioc)
            .serve(route::post<"/">([text](const request& req, std::string data)
                                        -> awaitable<response> {
              REQUIRE_EQ(req.header().get(http::field::content_type),
                         http::fields::content_type::plaintext());
              REQUIRE(!req.header().get(http::field::content_length));
              REQUIRE_EQ(data, text);
              co_return response::ok().build();
            }))
            .build();
  REQUIRE(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::post)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_header(http::field::connection, "close")
                  .set_header(http::field::content_type,
                              http::fields::content_type::plaintext())
                  .set_stream(async_readable_chunk_stream<5>(text))
                  .async_send();
        REQUIRE_EQ(res->status_code(), http::status::ok);
        REQUIRE_EQ(co_await res->as_string(), "");
      },
      net::use_future)
      .get();
}

TEST_SUITE_END();
