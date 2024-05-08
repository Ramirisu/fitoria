//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/cert.hpp>
#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/client.hpp>
#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;
using fitoria::client::http_client;

TEST_SUITE_BEGIN("[fitoria.web.http_server.tls]");

#if defined(FITORIA_HAS_OPENSSL)

void test_with_tls(net::ssl::context::method server_ssl_ver,
                   net::ssl::context::method client_ssl_ver)
{
  const auto port = generate_port();
  net::io_context ioc;
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/api/repos/{repo}">(
                [](request& req, std::string body) -> awaitable<response> {
                  CHECK_EQ(req.method(), http::verb::get);
                  CHECK_EQ(req.path().size(), 1);
                  CHECK_EQ(req.path().at("repo"), "fitoria");
                  CHECK_EQ(req.path().match_pattern(), "/api/repos/{repo}");
                  CHECK_EQ(req.path().match_path(), "/api/repos/fitoria");
                  CHECK_EQ(req.fields().get(http::field::content_type),
                           http::fields::content_type::plaintext());
                  CHECK_EQ(body, "hello world");
                  co_return response::ok().build();
                }))
            .build();
  auto ssl_ctx = cert::get_server_ssl_ctx(server_ssl_ver);
  server.bind_ssl(server_ip, port, ssl_ctx);

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
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
        CHECK_EQ(res->status_code(), http::status::ok);
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
  auto server = http_server_builder(ioc)
                    .set_tls_handshake_timeout(std::chrono::milliseconds(500))
                    .serve(route::get<"/">([]() -> awaitable<response> {
                      co_return response::ok().build();
                    }))
                    .build();
  auto ssl_ctx
      = cert::get_server_ssl_ctx(net::ssl::context::method::tls_server);
  server.bind_ssl(server_ip, port, ssl_ctx);

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        auto ssl_ctx
            = cert::get_client_ssl_ctx(net::ssl::context::method::tls_client);
        auto stream = ssl_stream(co_await net::this_coro::executor, ssl_ctx);
        CHECK(co_await get_lowest_layer(stream).async_connect(
            net::ip::tcp::endpoint(net::ip::make_address(server_ip), port),
            use_awaitable));

        auto timer = net::steady_timer(co_await net::this_coro::executor);
        timer.expires_after(std::chrono::seconds(5));
        co_await timer.async_wait(use_awaitable);

        CHECK(!(co_await stream.async_handshake(net::ssl::stream_base::client,
                                                use_awaitable)));
      },
      net::use_future)
      .get();
}

#endif

TEST_SUITE_END();
