//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria_certificate.h>
#include <fitoria_http_server_utils.h>
#include <fitoria_simple_http_client.h>

#include <fitoria/web.hpp>

#include <thread>

using namespace fitoria;

using namespace http_server_utils;

TEST_SUITE_BEGIN("web.http_server.tls");

#if defined(FITORIA_HAS_OPENSSL)

namespace {

void configure_server(http_server::builder& builder)
{
  builder.route(route(http::verb::get, "/api/repos/{repo}",
                      [](http_request& req) -> net::awaitable<http_response> {
                        CHECK_EQ(req.method(), http::verb::get);
                        CHECK_EQ(req.route_params().size(), 1);
                        CHECK_EQ(req.route_params().at("repo"), "fitoria");
                        CHECK_EQ(req.path(), "/api/repos/fitoria");
                        CHECK_EQ(req.headers().at(http::field::content_type),
                                 "text/plain");
                        CHECK_EQ(req.body(), "hello world");
                        co_return http_response(http::status::ok);
                      }));
}

void configure_client(simple_http_client& client)
{
  client.with(http::verb::get)
      .with_target("/api/repos/fitoria")
      .with_field(http::field::content_type, "text/plain")
      .with_body("hello world");
}

}

void test_with_tls(net::ssl::context::method server_ssl_ver,
                   net::ssl::context::method client_ssl_ver)
{
  const auto port = generate_port();
  auto server = http_server::builder().configure(configure_server).build();
  server.bind_ssl(server_ip, port, cert::get_server_ssl_ctx(server_ssl_ver));
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> net::awaitable<void> { co_await server.async_run(); },
      net::detached);
  std::jthread thread([&]() { ioc.run(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto client = simple_http_client(localhost, port);
  configure_client(client);
  auto resp = client.send_request(cert::get_client_ssl_ctx(client_ssl_ver));
  CHECK_EQ(resp.result(), http::status::ok);
  ioc.stop();
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

#endif

TEST_SUITE_END();
