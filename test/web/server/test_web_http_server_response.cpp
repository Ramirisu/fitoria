//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/async_readable_chunk_stream.hpp>
#include <fitoria/test/http_client.hpp>
#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/web.hpp>

#include <boost/scope/scope_exit.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.http_server.response]");

TEST_CASE("response status only")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">([]() -> awaitable<response> {
                      co_return response::no_content().build();
                    }))
                    .build();
  REQUIRE(server.bind(localhost, port));

  auto worker = std::thread([&]() { ioc.run(); });
  auto guard = boost::scope::make_scope_exit([&]() {
    ioc.stop();
    worker.join();
  });
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
        REQUIRE_EQ(res->status(), http::status::no_content);
        REQUIRE_EQ(res->headers().get(http::field::connection), "close");
        REQUIRE(!res->headers().get(http::field::content_length));
        REQUIRE_EQ(co_await async_read_until_eof<std::string>(res->body()),
                   std::string());
        REQUIRE_EQ(co_await res->as_string(), std::string());
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
      = http_server::builder(ioc)
            .serve(route::get<"/">([text]() -> awaitable<response> {
              co_return response::ok()
                  .set_header(http::field::content_type, mime::text_plain())
                  .set_body(text);
            }))
            .build();
  REQUIRE(server.bind(localhost, port));

  auto worker = std::thread([&]() { ioc.run(); });
  auto guard = boost::scope::make_scope_exit([&]() {
    ioc.stop();
    worker.join();
  });
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
        REQUIRE_EQ(res->status(), http::status::ok);
        REQUIRE_EQ(res->headers().get(http::field::connection), "close");
        REQUIRE_EQ(res->headers().get(http::field::content_type),
                   mime::text_plain());
        REQUIRE_EQ(res->headers().get(http::field::content_length),
                   std::to_string(text.size()));
        REQUIRE_EQ(co_await res->as_string(), text);
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
      = http_server::builder(ioc)
            .serve(route::get<"/">([text]() -> awaitable<response> {
              co_return response::ok()
                  .set_header(http::field::content_type, mime::text_plain())
                  .set_stream_body(async_readable_chunk_stream<5>(text));
            }))
            .build();
  REQUIRE(server.bind(localhost, port));

  auto worker = std::thread([&]() { ioc.run(); });
  auto guard = boost::scope::make_scope_exit([&]() {
    ioc.stop();
    worker.join();
  });
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
        REQUIRE_EQ(res->status(), http::status::ok);
        REQUIRE_EQ(res->headers().get(http::field::content_type),
                   mime::text_plain());
        REQUIRE(!res->headers().get(http::field::content_length));
        REQUIRE_EQ(co_await res->as_string(), text);
      },
      net::use_future)
      .get();
}

TEST_CASE("response with default HTTP/1.1")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">([]() -> awaitable<response> {
                      co_return response::ok().build();
                    }))
                    .build();
  REQUIRE(server.bind(localhost, port));

  auto worker = std::thread([&]() { ioc.run(); });
  auto guard = boost::scope::make_scope_exit([&]() {
    ioc.stop();
    worker.join();
  });
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
        REQUIRE_EQ(res->status(), http::status::ok);
        REQUIRE_EQ(res->headers().get(http::field::connection), "close");
        REQUIRE_EQ(res->version(), http::version::v1_1);
      },
      net::use_future)
      .get();
}

TEST_CASE("response with HTTP/1.0")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">([]() -> awaitable<response> {
              co_return response::ok().set_version(http::version::v1_0).build();
            }))
            .build();
  REQUIRE(server.bind(localhost, port));

  auto worker = std::thread([&]() { ioc.run(); });
  auto guard = boost::scope::make_scope_exit([&]() {
    ioc.stop();
    worker.join();
  });
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
        REQUIRE_EQ(res->status(), http::status::ok);
        REQUIRE(!res->headers().get(http::field::connection));
        REQUIRE_EQ(res->version(), http::version::v1_0);
      },
      net::use_future)
      .get();
}

TEST_SUITE_END();
