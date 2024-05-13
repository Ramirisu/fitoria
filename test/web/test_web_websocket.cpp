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

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.websocket]");

auto server_handler(websocket::context& ctx) -> awaitable<void>
{
  bool close_called = false;

  auto msg = co_await ctx.async_read();
  for (int seq = 0; msg; seq += 2) {
    if (std::visit(
            overloaded {
                [&seq](websocket::binary_t binary) {
                  CHECK(seq % 4 == 0);
                  CHECK(range_equal(
                      binary.value,
                      fmt::format("sequence number: {}", seq),
                      [](auto l, auto r) { return to_underlying(l) == r; }));
                  return false;
                },
                [&seq](websocket::text_t text) {
                  CHECK(seq % 4 == 2);
                  CHECK_EQ(text.value, fmt::format("sequence number: {}", seq));
                  return false;
                },
                [&seq, &close_called](websocket::close_t) {
                  CHECK(seq == 100);
                  close_called = true;
                  return true;
                } },
            *msg)) {
      break;
    }

    CHECK(co_await ctx.async_ping());

    const auto out = fmt::format("sequence number: {}", seq + 1);
    if (seq % 4 == 0) {
      CHECK(co_await ctx.async_write_text(out));
    } else {
      CHECK(co_await ctx.async_write_binary(
          std::span { out.data(), out.size() }));
    }

    msg = co_await ctx.async_read();
  }

  CHECK(close_called);
}

template <typename Stream>
auto client_handler(Stream& stream) -> awaitable<void>
{
  for (int seq = 0; seq < 100; seq += 2) {
    const auto out = fmt::format("sequence number: {}", seq);
    if (seq % 4 == 0) {
      stream.binary(true);
      CHECK(co_await stream.async_write(
          net::const_buffer(out.data(), out.size()), use_awaitable));
    } else {
      stream.text(true);
      CHECK(co_await stream.async_write(
          net::const_buffer(out.data(), out.size()), use_awaitable));
    }

    dynamic_buffer<std::string> buffer;
    CHECK(co_await stream.async_read(buffer, use_awaitable));
    CHECK_EQ(buffer.release(), fmt::format("sequence number: {}", seq + 1));
  }

  co_await stream.async_close(boost::beast::websocket::close_code::none,
                              use_awaitable);
}

TEST_CASE("websocket")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/">([](websocket ws) -> awaitable<response> {
              co_return ws.set_handler(server_handler);
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
        auto stream = boost::beast::websocket::stream<boost::beast::tcp_stream>(
            co_await net::this_coro::executor);
        CHECK(co_await boost::beast::get_lowest_layer(stream).async_connect(
            net::ip::tcp::endpoint(net::ip::make_address(server_ip), port),
            use_awaitable));
        CHECK(co_await stream.async_handshake("localhost", "/", use_awaitable));

        co_await client_handler(stream);
      },
      net::use_future)
      .get();
}

#if defined(FITORIA_HAS_OPENSSL)

TEST_CASE("secure websocket")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/">([](websocket ws) -> awaitable<response> {
              co_return ws.set_handler(server_handler);
            }))
            .build();
  auto ssl_ctx = cert::get_server_ssl_ctx(net::ssl::context_base::tls_server);
  server.bind_ssl(server_ip, port, ssl_ctx);

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        auto ssl_ctx
            = cert::get_client_ssl_ctx(net::ssl::context_base::tls_client);
        auto stream = boost::beast::websocket::stream<
            boost::beast::ssl_stream<boost::beast::tcp_stream>>(
            co_await net::this_coro::executor, ssl_ctx);
        CHECK(co_await boost::beast::get_lowest_layer(stream).async_connect(
            net::ip::tcp::endpoint(net::ip::make_address(server_ip), port),
            use_awaitable));
        CHECK(co_await stream.next_layer().async_handshake(
            net::ssl::stream_base::client, use_awaitable));
        CHECK(co_await stream.async_handshake("localhost", "/", use_awaitable));

        co_await client_handler(stream);
      },
      net::use_future)
      .get();
}

#endif

TEST_CASE("async_close")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/">([](websocket ws) -> awaitable<response> {
              co_return ws.set_handler(
                  [](websocket::context& ctx) -> awaitable<void> {
                    co_await ctx.async_close(websocket::close_code::normal);
                  });
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
        auto stream = boost::beast::websocket::stream<boost::beast::tcp_stream>(
            co_await net::this_coro::executor);
        CHECK(co_await boost::beast::get_lowest_layer(stream).async_connect(
            net::ip::tcp::endpoint(net::ip::make_address(server_ip), port),
            use_awaitable));
        CHECK(co_await stream.async_handshake("localhost", "/", use_awaitable));

        dynamic_buffer<std::string> buffer;
        auto result = co_await stream.async_read(buffer, use_awaitable);
        CHECK_EQ(result.error(),
                 make_error_code(boost::beast::websocket::error::closed));
      },
      net::use_future)
      .get();
}

TEST_SUITE_END();
