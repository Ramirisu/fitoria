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

#include <fitoria/web.hpp>

#include <boost/scope/scope_exit.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.http_server.tls]");

#if defined(FITORIA_HAS_OPENSSL)

void test_with_tls(net::ssl::context::method server_ssl_ver,
                   net::ssl::context::method client_ssl_ver)
{
  const auto port = generate_port();
  net::io_context ioc;
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/api/repos/{repo}">(
                [](request& req, std::string body) -> awaitable<response> {
                  REQUIRE_EQ(req.method(), http::verb::get);
                  REQUIRE_EQ(req.path().size(), 1);
                  REQUIRE_EQ(req.path().at("repo"), "fitoria");
                  REQUIRE_EQ(req.path().match_pattern(), "/api/repos/{repo}");
                  REQUIRE_EQ(req.path().match_path(), "/api/repos/fitoria");
                  REQUIRE_EQ(req.headers().get(http::field::content_type),
                             mime::text_plain());
                  REQUIRE_EQ(body, "hello world");
                  co_return response::ok().build();
                }))
            .build();
  auto ssl_ctx = cert::get_server_ssl_ctx(server_ssl_ver);
  server.bind(localhost, port, ssl_ctx);

  auto worker = std::thread([&]() { ioc.run(); });
  auto guard = boost::scope::make_scope_exit([&]() {
    ioc.stop();
    worker.join();
  });
  ;
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        auto ssl_ctx = cert::get_client_ssl_ctx(client_ssl_ver);
        auto res
            = co_await http_client()
                  .set_method(http::verb::get)
                  .set_url(to_local_url(
                      boost::urls::scheme::https, port, "/api/repos/fitoria"))
                  .set_plaintext("hello world")
                  .async_send(ssl_ctx);
        REQUIRE_EQ(res->status(), http::status::ok);
      },
      net::use_future)
      .get();
}

#if OPENSSL_VERSION_MAJOR < 3

TEST_CASE("tls/tlsv1")
{
  using net::ssl::context;
  test_with_tls(context::method::tls_server, context::method::tlsv1_client);
}

TEST_CASE("tls/tlsv11")
{
  using net::ssl::context;
  test_with_tls(context::method::tls_server, context::method::tlsv11_client);
}

#endif

TEST_CASE("tls/tlsv12")
{
  using net::ssl::context;
  test_with_tls(context::method::tls_server, context::method::tlsv12_client);
}

TEST_CASE("tls/tlsv13")
{
  using net::ssl::context;
  test_with_tls(context::method::tls_server, context::method::tlsv13_client);
}

TEST_CASE("tls_handshake_timeout")
{
  const auto port = generate_port();
  net::io_context ioc;
  auto server = http_server::builder(ioc)
                    .set_tls_handshake_timeout(std::chrono::milliseconds(500))
                    .serve(route::get<"/">([]() -> awaitable<response> {
                      co_return response::ok().build();
                    }))
                    .build();
  auto ssl_ctx
      = cert::get_server_ssl_ctx(net::ssl::context::method::tls_server);
  server.bind(localhost, port, ssl_ctx);

  auto worker = std::thread([&]() { ioc.run(); });
  auto guard = boost::scope::make_scope_exit([&]() {
    ioc.stop();
    worker.join();
  });
  ;
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        auto ssl_ctx
            = cert::get_client_ssl_ctx(net::ssl::context::method::tls_client);
        auto stream = ssl_stream<net::ip::tcp>(
            co_await net::this_coro::executor, ssl_ctx);
        REQUIRE(co_await get_lowest_layer(stream).async_connect(
            net::ip::tcp::endpoint(net::ip::make_address(localhost), port),
            use_awaitable));

        auto timer = net::steady_timer(co_await net::this_coro::executor);
        timer.expires_after(std::chrono::seconds(5));
        co_await timer.async_wait(use_awaitable);

        REQUIRE(!(co_await stream.async_handshake(net::ssl::stream_base::client,
                                                  use_awaitable)));
      },
      net::use_future)
      .get();
}

#endif

TEST_SUITE_END();
