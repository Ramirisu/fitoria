//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/client.hpp>
#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;
using fitoria::client::http_client;

TEST_SUITE_BEGIN("[fitoria.web.http_server.keep_alive]");

TEST_CASE("request with keep-alive")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(
                route::post<"/">([](std::string body) -> awaitable<response> {
                  co_return response::ok()
                      .set_header(http::field::content_type,
                                  http::fields::content_type::plaintext())
                      .set_body(body);
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
        auto stream = tcp_stream(co_await net::this_coro::executor);
        CHECK(co_await stream.async_connect(
            net::ip::tcp::endpoint(net::ip::make_address(server_ip), port),
            use_awaitable));

        // write multiple requests at once for pipeline simulation
        for (std::size_t i = 0; i < 10; ++i) {
          namespace http = boost::beast::http;
          auto req
              = http::request<http::string_body>(http::verb::post, "/", 11);
          req.keep_alive(true);
          req.insert(http::field::content_type, "text/plain");
          req.body() = fmt::format("sequence: {}", i);
          req.prepare_payload();
          CHECK(co_await http::async_write(stream, req, use_awaitable));
        }

        // then receive all the responses
        auto buffer = flat_buffer();
        for (std::size_t i = 0; i < 10; ++i) {
          namespace http = boost::beast::http;
          auto res = http::response<http::string_body>();
          CHECK(co_await http::async_read(stream, buffer, res, use_awaitable));
          CHECK(res.keep_alive());
          CHECK_EQ(res.at(http::field::content_type), "text/plain");
          CHECK_EQ(res.body(), fmt::format("sequence: {}", i));
        }
      },
      net::use_future)
      .get();
}

TEST_SUITE_END();
