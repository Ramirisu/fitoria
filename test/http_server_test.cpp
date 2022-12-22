//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the ramirisu Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.ramirisu.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/core/http.hpp>
#include <fitoria/http_server/http_server.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("http_server");

namespace {

class simple_http_client {
public:
  simple_http_client(std::string host, std::uint16_t port)
      : host_(std::move(host))
      , port_(port)
  {
  }

  simple_http_client& with(methods method)
  {
    method_ = method;
    return *this;
  }

  simple_http_client& with_target(std::string target)
  {
    target_ = std::move(target);
    return *this;
  }

  simple_http_client& with_body(std::string body)
  {
    body_ = std::move(body);
    return *this;
  }

  auto send_request()
  {
    net::io_context ioc;
    auto future = net::co_spawn(
        ioc, do_session(host_, std::to_string(port_), method_, target_, body_),
        net::use_future);
    ioc.run();

    return future.get();
  }

private:
  static net::awaitable<http::response<http::dynamic_body>>
  do_session(std::string host,
             std::string port,
             methods method,
             std::string target,
             std::string body)
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
    req.body() = std::move(body);
    req.prepare_payload();

    stream.expires_after(std::chrono::seconds(5));

    co_await http::async_write(stream, req);

    net::flat_buffer buffer;

    http::response<http::dynamic_body> res;

    co_await http::async_read(stream, buffer, res);

    net::error_code ec;
    stream.socket().shutdown(net::ip::tcp::socket::shutdown_both, ec);

    if (ec && ec != net::errc::not_connected) {
      throw boost::system::system_error(ec, "shutdown");
    }

    co_return res;
  }

  std::string host_;
  std::uint16_t port_;
  methods method_;
  std::string target_;
  std::string body_;
};

const char* localhost = "127.0.0.1";
const std::uint16_t port = 8080;

}

TEST_CASE("middlewares and router's invocation order")
{
  int state = 0;
  http_server server;
  server.route(
      http_server::router_group_type("/api")
          .use([&](auto& ctx) -> net::awaitable<void> {
            CHECK_EQ(++state, 1);
            co_await ctx.next();
            CHECK_EQ(++state, 5);
          })
          .use([&](auto& ctx) -> net::awaitable<void> {
            CHECK_EQ(++state, 2);
            co_await ctx.next();
            CHECK_EQ(++state, 4);
          })
          .route(methods::get, "/get",
                 [&]([[maybe_unused]] auto& ctx) -> net::awaitable<void> {
                   CHECK_EQ(++state, 3);
                   co_return;
                 }));
  server.run(localhost, port);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  auto resp = simple_http_client(localhost, port)
                  .with(methods::get)
                  .with_target("/api/get")
                  .send_request();
  CHECK_EQ(resp.result_int(), 200);

  CHECK_EQ(++state, 6);
}

TEST_CASE("request")
{
  http_server server;
  server.route(
      { methods::get, "/api/get", [&](auto& ctx) -> net::awaitable<void> {
         auto req = ctx.request();
         CHECK_EQ(req.method(), methods::get);
         CHECK_EQ(req.body(), "text");
         co_return;
       } });
  server.run(localhost, port);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  auto resp = simple_http_client(localhost, port)
                  .with(methods::get)
                  .with_target("/api/get")
                  .with_body("text")
                  .send_request();
  CHECK_EQ(resp.result_int(), 200);
}

TEST_SUITE_END();
