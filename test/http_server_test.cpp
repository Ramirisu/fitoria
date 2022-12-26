//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria_certificate.h>
#include <fitoria_http_server_utils.h>
#include <fitoria_simple_http_client.h>

#include <fitoria/core/http.hpp>
#include <fitoria/http_server/http_server.hpp>

using namespace fitoria;

using namespace http_server_utils;

TEST_SUITE_BEGIN("http_server");

TEST_CASE("http_server_config")
{
  const auto port = generate_port();
  auto server = http_server(
      http_server_config()
          .set_threads(1)
          .set_max_listen_connections(2048)
          .set_client_request_timeout(std::chrono::seconds(1))
          .route(router(verb::get, "/api/get",
                        [&]([[maybe_unused]] http_context& c)
                            -> net::awaitable<void> { co_return; })));
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(verb::get)
                  .with_target("/api/get")
                  .send_request();
  CHECK_EQ(resp.result(), status::ok);
}

TEST_CASE("middlewares and router's invocation order")
{
  int state = 0;
  const auto port = generate_port();
  auto server = http_server(http_server_config().route(
      router_group("/api")
          .use([&](auto& c) -> net::awaitable<void> {
            CHECK_EQ(++state, 1);
            co_await c.next();
            CHECK_EQ(++state, 5);
          })
          .use([&](auto& c) -> net::awaitable<void> {
            CHECK_EQ(++state, 2);
            co_await c.next();
            CHECK_EQ(++state, 4);
          })
          .route(verb::get, "/get",
                 [&]([[maybe_unused]] http_context& c) -> net::awaitable<void> {
                   CHECK_EQ(++state, 3);
                   co_return;
                 })));
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(verb::get)
                  .with_target("/api/get")
                  .with_field(field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), status::ok);

  CHECK_EQ(++state, 6);
}

namespace simple_http_request_test {

void configure_server(http_server_config& config)
{
  config.route(
      router(verb::get, "/api/v1/users/{user}/filmography/years/{year}",
             [&](http_context& c, http_route& route,
                 http_request& req) -> net::awaitable<void> {
               auto test_route = [](http_route& route) {
                 CHECK_EQ(route.path(),
                          "/api/v1/users/{user}/filmography/years/{year}");

                 CHECK_EQ(route.at("user"), "Rina Hikada");
                 CHECK_EQ(route.at("year"), "2022");
               };

               test_route(c.route());
               test_route(route);

               auto test_request = [](http_request& req) {
                 CHECK_EQ(req.method(), verb::get);
                 CHECK_EQ(req.path(),
                          "/api/v1/users/Rina Hikada/filmography/years/2022");
                 CHECK_EQ(req.params().size(), 2);
                 CHECK_EQ(req.params().at("name"), "Rina Hikada");
                 CHECK_EQ(req.params().at("birth"), "1994/06/15");

                 CHECK_EQ(req.body(),
                          json::serialize(json::value {
                              { "name", "Rina Hikada" },
                              { "birth", "1994/06/15" },
                          }));
               };

               test_request(c.request());
               test_request(req);
               co_return;
             }));
}

void configure_client(simple_http_client& client)
{
  client.with(verb::get)
      .with_target(
          R"(/api/v1/users/Rina%20Hikada/filmography/years/2022?name=Rina%20Hikada&birth=1994%2F06%2F15)")
      .with_field(field::content_type, "application/json")
      .with_field(field::content_type, "charset=utf-8")
      .with_body(json::serialize(json::value {
          { "name", "Rina Hikada" },
          { "birth", "1994/06/15" },
      }))
      .with_field(field::connection, "close");
}

TEST_CASE("simple request without tls")
{
  const auto port = generate_port();
  auto server = http_server(http_server_config().configure(configure_server));
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto client = simple_http_client(localhost, port);
  configure_client(client);
  auto resp = client.send_request();
  CHECK_EQ(resp.result(), status::ok);
}

#if defined(FITORIA_HAS_OPENSSL)

void test_with_tls(net::ssl::context::method server_ssl_ver,
                   net::ssl::context::method client_ssl_ver)
{
  const auto port = generate_port();
  auto server = http_server(http_server_config().configure(configure_server));
  server.bind_ssl(server_ip, port, cert::get_server_ssl_ctx(server_ssl_ver))
      .run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto client = simple_http_client(localhost, port);
  configure_client(client);
  auto resp = client.send_request(cert::get_client_ssl_ctx(client_ssl_ver));
  CHECK_EQ(resp.result(), status::ok);
}

TEST_CASE("simple request with tls/tlsv12")
{
  using net::ssl::context;
  test_with_tls(context::method::tls_server, context::method::tlsv12_client);
}

#if 0
// TODO: tlsv13 client throws bad version error

TEST_CASE("simple request with tls/tlsv13")
{
  using net::ssl::context;
  test_with_tls(context::method::tls_server, context::method::tlsv13_client);
}

#endif

#endif

}

TEST_CASE("response status only")
{
  const auto port = generate_port();
  auto server = http_server(http_server_config().route(
      router(verb::get, "/api", [](http_context& c) -> net::awaitable<void> {
        c.status(status::accepted);
        co_return;
      })));
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(verb::get)
                  .with_target("/api")
                  .with_field(field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), status::accepted);
  CHECK_EQ(resp.at(field::connection), "close");
  CHECK_EQ(resp.at(field::content_length), "0");
  CHECK_EQ(resp.body(), "");
}

TEST_CASE("response with plain text")
{
  const auto port = generate_port();
  auto server = http_server(http_server_config().route(
      router(verb::get, "/api", [](http_context& c) -> net::awaitable<void> {
        c.plain_text("plain text");
        co_return;
      })));
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(verb::get)
                  .with_target("/api")
                  .with_field(field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), status::ok);
  CHECK_EQ(resp.at(field::content_type), "text/plain; charset=utf-8");
  CHECK_EQ(resp.body(), "plain text");
}

TEST_CASE("response with json")
{
  const auto port = generate_port();
  auto server = http_server(http_server_config().route(
      router(verb::get, "/api", [](http_context& c) -> net::awaitable<void> {
        c.json({
            { "obj_boolean", true },
            { "obj_number", 1234567 },
            { "obj_string", "str" },
            { "obj_array", json::array { false, 7654321, "rts" } },
        });
        co_return;
      })));
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(verb::get)
                  .with_target("/api")
                  .with_field(field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), status::ok);
  CHECK_EQ(resp.at(field::content_type), "application/json; charset=utf-8");
  CHECK_EQ(resp.body(),
           json::serialize(json::value({
               { "obj_boolean", true },
               { "obj_number", 1234567 },
               { "obj_string", "str" },
               { "obj_array", json::array { false, 7654321, "rts" } },
           })));
}

TEST_SUITE_END();
