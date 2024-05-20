//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/async_readable_chunk_stream.hpp>
#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/client.hpp>
#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;
using fitoria::client::http_client;

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
  CHECK(server.bind(server_ip, port));

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
        CHECK_EQ(res->status_code(), http::status::no_content);
        CHECK_EQ(res->header().get(http::field::connection), "close");
        CHECK(!res->header().get(http::field::content_length));
        CHECK(!(co_await async_read_until_eof<std::string>(res->body())));
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
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">([text]() -> awaitable<response> {
                      co_return response::ok()
                          .set_header(http::field::content_type,
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
      [&]() -> awaitable<void> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::get)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_header(http::field::connection, "close")
                  .async_send();
        CHECK_EQ(res->status_code(), http::status::ok);
        CHECK_EQ(res->header().get(http::field::connection), "close");
        CHECK_EQ(res->header().get(http::field::content_type),
                 http::fields::content_type::plaintext());
        CHECK_EQ(res->header().get(http::field::content_length),
                 std::to_string(text.size()));
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
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">([text]() -> awaitable<response> {
                      co_return response::ok()
                          .set_header(http::field::content_type,
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
      [&]() -> awaitable<void> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::get)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_header(http::field::connection, "close")
                  .async_send();
        CHECK_EQ(res->status_code(), http::status::ok);
        CHECK_EQ(res->header().get(http::field::content_type),
                 http::fields::content_type::plaintext());
        CHECK(!res->header().get(http::field::content_length));
        CHECK_EQ(co_await res->as_string(), text);
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
  CHECK(server.bind(server_ip, port));

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
        CHECK_EQ(res->status_code(), http::status::ok);
        CHECK_EQ(res->header().get(http::field::connection), "close");
        CHECK_EQ(res->version(), http::version::v1_1);
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
  CHECK(server.bind(server_ip, port));

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
        CHECK_EQ(res->status_code(), http::status::ok);
        CHECK(!res->header().get(http::field::connection));
        CHECK_EQ(res->version(), http::version::v1_0);
      },
      net::use_future)
      .get();
}

TEST_SUITE_END();
