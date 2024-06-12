//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/cert.hpp>
#include <fitoria/test/http_client.hpp>
#include <fitoria/test/http_server_utils.hpp>
#include <fitoria/test/utility.hpp>

#include <fitoria/web.hpp>

#include <boost/scope/scope_exit.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.http_server.bind_local]");

TEST_CASE("bind_local")
{
  const auto file_path = get_random_temp_file_path();

  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(
                route::post<"/">([=](const http::header_map& headers,
                                     std::string body) -> awaitable<response> {
                  REQUIRE_EQ(headers.get(http::field::content_type),
                             mime::text_plain());
                  REQUIRE_EQ(body, "Hello World!");
                  co_return response::ok()
                      .set_header(http::field::content_type, mime::text_plain())
                      .set_body("Hello World!");
                }))
            .build();
  REQUIRE(server.bind_local(file_path));

  auto worker = std::thread([&]() { ioc.run(); });
  auto guard = boost::scope::make_scope_exit([&]() {
    ioc.stop();
    worker.join();
  });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        namespace http = boost::beast::http;

        auto stream = basic_stream<net::local::stream_protocol>(
            co_await net::this_coro::executor);
        REQUIRE(co_await stream.async_connect(
            net::local::stream_protocol::endpoint(file_path), use_awaitable));
        stream.expires_after(std::chrono::seconds(5));
        auto req = http::request<http::string_body>(http::verb::post, "/", 11);
        req.keep_alive(false);
        req.insert(http::field::content_type, "text/plain");
        req.body() = "Hello World!";
        req.prepare_payload();
        REQUIRE(co_await http::async_write(stream, req, use_awaitable));

        auto buffer = flat_buffer();
        auto res = http::response<http::string_body>();
        REQUIRE(co_await http::async_read(stream, buffer, res, use_awaitable));
        REQUIRE_EQ(res.at(http::field::content_type), "text/plain");
        REQUIRE_EQ(res.body(), "Hello World!");
      },
      net::use_future)
      .get();
}

#if defined(FITORIA_HAS_OPENSSL)

TEST_CASE("bind_local TLS")
{
  const auto file_path = get_random_temp_file_path();

  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(
                route::post<"/">([=](const http::header_map& headers,
                                     std::string body) -> awaitable<response> {
                  REQUIRE_EQ(headers.get(http::field::content_type),
                             mime::text_plain());
                  REQUIRE_EQ(body, "Hello World!");
                  co_return response::ok()
                      .set_header(http::field::content_type, mime::text_plain())
                      .set_body("Hello World!");
                }))
            .build();
  auto ssl_ctx = cert::get_server_ssl_ctx(net::ssl::context::tls_server);
  REQUIRE(server.bind_local(file_path, ssl_ctx));

  auto worker = std::thread([&]() { ioc.run(); });
  auto guard = boost::scope::make_scope_exit([&]() {
    ioc.stop();
    worker.join();
  });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        namespace http = boost::beast::http;

        auto ssl_ctx = cert::get_client_ssl_ctx(net::ssl::context::tls_client);
        auto stream = ssl_stream<net::local::stream_protocol>(
            co_await net::this_coro::executor, ssl_ctx);
        REQUIRE(co_await get_lowest_layer(stream).async_connect(
            net::local::stream_protocol::endpoint(file_path), use_awaitable));
        REQUIRE(co_await stream.async_handshake(net::ssl::stream_base::client,
                                                use_awaitable));
        get_lowest_layer(stream).expires_after(std::chrono::seconds(5));
        auto req = http::request<http::string_body>(http::verb::post, "/", 11);
        req.keep_alive(false);
        req.insert(http::field::content_type, "text/plain");
        req.body() = "Hello World!";
        req.prepare_payload();
        REQUIRE(co_await http::async_write(stream, req, use_awaitable));

        auto buffer = flat_buffer();
        auto res = http::response<http::string_body>();
        REQUIRE(co_await http::async_read(stream, buffer, res, use_awaitable));
        REQUIRE_EQ(res.at(http::field::content_type), "text/plain");
        REQUIRE_EQ(res.body(), "Hello World!");
      },
      net::use_future)
      .get();
}

#endif

TEST_SUITE_END();
