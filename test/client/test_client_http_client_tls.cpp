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

#include <boost/scope/scope_exit.hpp>

using namespace fitoria;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.client.http_client.tls]");

#if defined(FITORIA_HAS_OPENSSL)

TEST_CASE("async_send")
{
  auto get_certs = []() {
    auto ssl_ctx = net::ssl::context(net::ssl::context::method::tls);
    ssl_ctx.set_verify_mode(net::ssl::verify_peer);
    cacert::add_builtin_cacerts(ssl_ctx);
    return ssl_ctx;
  };

  sync_wait([&]() -> awaitable<void> {
    auto ssl_ctx = get_certs();
    auto res = co_await http_client()
                   .set_method(http::verb::get)
                   .set_url("https://httpbun.com/get")
                   .async_send(ssl_ctx);
    REQUIRE_EQ(res->status().value(), http::status::ok);
    REQUIRE(!(co_await res->as_string())->empty());
  });

  sync_wait([&]() -> awaitable<void> {
    auto ssl_ctx = get_certs();
    REQUIRE(!(co_await http_client()
                  .set_method(http::verb::get)
                  .set_url("")
                  .async_send(ssl_ctx)));
  });
}

TEST_CASE("handshake_timeout")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        auto acceptor
            = socket_acceptor<net::ip::tcp>(co_await net::this_coro::executor);
        acceptor.open(net::ip::tcp::v4());
        acceptor.set_option(net::socket_base::reuse_address(true));
        acceptor.bind(
            net::ip::tcp::endpoint(net::ip::make_address(localhost), port));
        acceptor.listen();
        auto socket = co_await acceptor.async_accept(use_awaitable);
        REQUIRE(socket);

        auto timer = net::steady_timer(co_await net::this_coro::executor);
        timer.expires_after(std::chrono::seconds(5));
        co_await timer.async_wait(use_awaitable);

        socket->close();
      },
      net::detached);

  auto worker = std::thread([&]() { ioc.run(); });
  auto guard = boost::scope::make_scope_exit([&]() {
    ioc.stop();
    worker.join();
  });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        auto ssl_ctx = cert::get_client_ssl_ctx(net::ssl::context::tls_client);
        auto res
            = co_await http_client()
                  .set_handshake_timeout(std::chrono::seconds(1))
                  .set_method(http::verb::get)
                  .set_url(to_local_url(boost::urls::scheme::https, port, "/"))
                  .async_send(ssl_ctx);
        REQUIRE(!res);
        REQUIRE_EQ(res.error(), make_error_code(net::error::timed_out));
      },
      net::use_future)
      .get();
}

#endif

TEST_SUITE_END();
