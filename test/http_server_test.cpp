//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the ramirisu Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.ramirisu.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/core/methods.hpp>
#include <fitoria/http_server/http_server.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("http_server");

namespace {

net::awaitable<http::response<http::dynamic_body>> do_session(
    std::string host, std::string port, methods method, std::string target)
{
  auto resolver = net::use_awaitable.as_default_on(
      net::ip::tcp::resolver(co_await net::this_coro::executor));
  auto stream = net::use_awaitable.as_default_on(
      net::tcp_stream(co_await net::this_coro::executor));

  const auto results = co_await resolver.async_resolve(host, port);

  stream.expires_after(std::chrono::seconds(5));

  co_await stream.async_connect(results);

  http::request<http::string_body> req { method, target, 11 };
  req.set(http::field::host, host);

  stream.expires_after(std::chrono::seconds(5));

  co_await http::async_write(stream, req);

  net::flat_buffer b;

  http::response<http::dynamic_body> res;

  co_await http::async_read(stream, b, res);

  net::error_code ec;
  stream.socket().shutdown(net::ip::tcp::socket::shutdown_both, ec);

  if (ec && ec != net::errc::not_connected) {
    throw boost::system::system_error(ec, "shutdown");
  }

  co_return res;
}

auto send_request(std::string host,
                  std::uint16_t port,
                  methods method,
                  std::string target)
{
  net::io_context ioc;
  auto future = net::co_spawn(
      ioc, do_session(host, std::to_string(port), method, target),
      net::use_future);
  ioc.run();

  return future.get();
}

const char* localhost = "127.0.0.1";
const std::uint16_t port = 8080;
}

TEST_CASE("connection")
{
  int state = 0;
  http_server server(12);
  server.route(
      http_server::router_group_type("/api")
          .use([&](auto& ctx) {
            CHECK_EQ(++state, 1);
            ctx.next();
            CHECK_EQ(++state, 3);
          })
          .route(methods::get, "/get", [&](auto&) { CHECK_EQ(++state, 2); }));
  server.run(localhost, port);

  auto resp = send_request(localhost, port, methods::get, "/api/get");
  CHECK_EQ(resp.result_int(), 200);

  CHECK_EQ(++state, 4);
}

TEST_SUITE_END();
