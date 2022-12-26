//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria_certificate.h>

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

  simple_http_client& with(verb method)
  {
    method_ = method;
    return *this;
  }

  simple_http_client& with_target(std::string target)
  {
    target_ = std::move(target);
    return *this;
  }

  simple_http_client& with_field(field name, std::string_view value)
  {
    fields_.insert(name, value);
    return *this;
  }

  simple_http_client& with_body(std::string body)
  {
    body_ = std::move(body);
    return *this;
  }

  simple_http_client& set_request_timeout(std::chrono::milliseconds timeout)
  {
    request_timeout_ = timeout;
    return *this;
  }

  auto send_request()
  {
    net::io_context ioc;
    auto future = net::co_spawn(ioc, do_session(), net::use_future);
    ioc.run();

    return future.get();
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto send_request(net::ssl::context ssl_ctx)
  {
    net::io_context ioc;
    auto future
        = net::co_spawn(ioc, do_session(std::move(ssl_ctx)), net::use_future);
    ioc.run();

    return future.get();
  }
#endif

private:
  using resolver_t = typename net::ip::tcp::resolver::template rebind_executor<
      net::use_awaitable_t<>::executor_with_default<net::any_io_executor>>::
      other;

  net::awaitable<http::response<http::string_body>> do_session()
  {
    auto resolver = co_await new_resolver();
    auto stream = net::tcp_stream(co_await net::this_coro::executor);

    const auto results
        = co_await resolver.async_resolve(host_, std::to_string(port_));

    stream.expires_after(request_timeout_);

    co_await stream.async_connect(results);

    auto resp = co_await do_session_impl(stream);

    net::error_code ec;
    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);

    co_return resp;
  }

#if defined(FITORIA_HAS_OPENSSL)
  net::awaitable<http::response<http::string_body>>
  do_session(net::ssl::context ssl_ctx)
  {
    auto resolver = co_await new_resolver();
    auto stream = net::ssl_stream(co_await net::this_coro::executor, ssl_ctx);

    auto hostname = net::ip::host_name();
    SSL_set_tlsext_host_name(stream.native_handle(), hostname.c_str());

    const auto results
        = co_await resolver.async_resolve(host_, std::to_string(port_));

    net::get_lowest_layer(stream).expires_after(request_timeout_);

    co_await net::get_lowest_layer(stream).async_connect(results);

    net::get_lowest_layer(stream).expires_after(request_timeout_);

    co_await stream.async_handshake(net::ssl::stream_base::client);

    auto resp = co_await do_session_impl(stream.next_layer());

    net::error_code ec;
    stream.shutdown(ec);
    if (ec == net::error::eof || ec == net::ssl::error::stream_truncated) {
      ec.clear();
    }
    if (ec) {
      throw net::system_error(ec);
    }

    co_return resp;
  }
#endif

  net::awaitable<resolver_t> new_resolver()
  {
    co_return net::use_awaitable.as_default_on(
        net::ip::tcp::resolver(co_await net::this_coro::executor));
  }

  net::awaitable<http::response<http::string_body>>
  do_session_impl(net::tcp_stream& stream)
  {
    http::request<http::string_body> req { method_, target_, 11 };
    for (const auto& f : fields_) {
      req.set(f.name(), f.value());
    }
    req.set(http::field::host, host_);
    req.body() = body_;
    req.prepare_payload();

    stream.expires_after(request_timeout_);

    co_await http::async_write(stream, req);

    net::flat_buffer buffer;

    http::response<http::string_body> res;

    stream.expires_after(request_timeout_);

    co_await http::async_read(stream, buffer, res);

    co_return res;
  }

  std::string host_;
  std::uint16_t port_;
  verb method_ = verb::unknown;
  std::string target_;
  http::fields fields_;
  std::string body_;
  std::chrono::milliseconds request_timeout_ = std::chrono::seconds(5);
};

const auto server_start_wait_time = std::chrono::milliseconds(1000);
const char* server_ip = "127.0.0.1";
const char* localhost = "localhost";
std::uint16_t generate_port()
{
  static std::uint16_t port = 50000;
  return ++port;
}

}

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
  config.route(router(
      verb::get, "/api/v1/users/{user}/filmography/years/{year}",
      [&](http_context& c, http_request& req) -> net::awaitable<void> {
        CHECK_EQ(c.path(), "/api/v1/users/{user}/filmography/years/{year}");
        CHECK_EQ(c.encoded_params().size(), 2);
        CHECK_EQ((*c.encoded_params().find("user")).value, R"(Rina%20Hikada)");
        CHECK_EQ((*c.encoded_params().find("year")).value, R"(2022)");

        CHECK_EQ(c.params().size(), 2);
        CHECK_EQ((*c.params().find("user")).value, "Rina Hikada");
        CHECK_EQ((*c.params().find("year")).value, "2022");

        auto test_request = [](http_request& req) {
          CHECK_EQ(req.method(), verb::get);
          CHECK_EQ(req.encoded_path(),
                   R"(/api/v1/users/Rina%20Hikada/filmography/years/2022)");
          CHECK_EQ(req.path(),
                   "/api/v1/users/Rina Hikada/filmography/years/2022");
          CHECK_EQ(req.encoded_query(),
                   R"(name=Rina%20Hikada&birth=1994%2F06%2F15)");
          CHECK_EQ(req.query(), "name=Rina Hikada&birth=1994/06/15");
          CHECK_EQ(req.encoded_params().size(), 2);
          CHECK_EQ((*req.encoded_params().find("name")).value,
                   R"(Rina%20Hikada)");
          CHECK_EQ((*req.encoded_params().find("birth")).value,
                   R"(1994%2F06%2F15)");
          CHECK_EQ(req.params().size(), 2);
          CHECK_EQ((*req.params().find("name")).value, "Rina Hikada");
          CHECK_EQ((*req.params().find("birth")).value, "1994/06/15");

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

TEST_CASE("response with plain text")
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
