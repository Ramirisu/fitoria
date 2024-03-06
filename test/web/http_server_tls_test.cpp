//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria_certificate.h>
#include <fitoria_http_server_utils.h>

#include <fitoria/client.hpp>
#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace http_server_utils;
using fitoria::client::http_client;

TEST_SUITE_BEGIN("[fitoria.web.http_server.tls]");

#if defined(FITORIA_HAS_OPENSSL)

namespace {

void configure_server(http_server::builder& builder)
{
  builder.serve(route::get<"/api/repos/{repo}">(
      [](http_request& req, std::string body) -> lazy<http_response> {
        CHECK_EQ(req.method(), http::verb::get);
        CHECK_EQ(req.params().size(), 1);
        CHECK_EQ(req.params().at("repo"), "fitoria");
        CHECK_EQ(req.path(), "/api/repos/fitoria");
        CHECK_EQ(req.fields().get(http::field::content_type),
                 http::fields::content_type::plaintext());
        CHECK_EQ(body, "hello world");
        co_return http_response(http::status::ok);
      }));
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
      ioc, [&]() -> lazy<void> { co_await server.async_run(); }, net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> lazy<void> {
        auto res
            = (co_await http_client::get(
                   to_local_url(
                       boost::urls::scheme::https, port, "/api/repos/fitoria"))
                   .set_plaintext("hello world")
                   .async_send(cert::get_client_ssl_ctx(client_ssl_ver)))
                  .value();
        CHECK_EQ(res.status_code(), http::status::ok);
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

#endif

TEST_SUITE_END();
