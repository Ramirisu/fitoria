//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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

  simple_http_client& with_field(fields name, std::string_view value)
  {
    fields_.insert(name, value);
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
    auto future = net::co_spawn(ioc, do_session(), net::use_future);
    ioc.run();

    return future.get();
  }

  auto send_request(net::ssl::context ssl_ctx)
  {
    net::io_context ioc;
    auto future
        = net::co_spawn(ioc, do_session(std::move(ssl_ctx)), net::use_future);
    ioc.run();

    return future.get();
  }

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

    stream.expires_after(std::chrono::seconds(5));

    co_await stream.async_connect(results);

    auto resp = co_await do_session_impl(stream);

    net::error_code ec;
    stream.socket().shutdown(net::ip::tcp::socket::shutdown_both, ec);

    co_return resp;
  }

  net::awaitable<http::response<http::string_body>>
  do_session(net::ssl::context ssl_ctx)
  {
    auto resolver = co_await new_resolver();
    auto stream = net::ssl_stream(co_await net::this_coro::executor, ssl_ctx);

    const auto results
        = co_await resolver.async_resolve(host_, std::to_string(port_));

    net::get_lowest_layer(stream).expires_after(std::chrono::seconds(5));

    co_await net::get_lowest_layer(stream).async_connect(results);

    net::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

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

    stream.expires_after(std::chrono::seconds(5));

    co_await http::async_write(stream, req);

    net::flat_buffer buffer;

    http::response<http::string_body> res;

    co_await http::async_read(stream, buffer, res);

    co_return res;
  }

  std::string host_;
  std::uint16_t port_;
  methods method_ = methods::unknown;
  std::string target_;
  http::fields fields_;
  std::string body_;
};

net::ssl::context get_server_ssl_ctx()
{
  const std::string key
      = "-----BEGIN PRIVATE KEY-----\n"
        "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDEc3UP0Vcfp4pR\n"
        "J4JxLAcTyutWgbqaLup9TOc20i18DHcL5//mRkIAaNjsYuzIW2rvrcNy7I2cK6UQ\n"
        "jrZssu8vuNjyBFpfMQbsTTDK09QcNCfp96V88HZ1fQnEMlfaAlbrGcOGbV/rQyH6\n"
        "/eLbxA9+4MlzwAYlzub4enTCJGnBboSK3X5vEWW6UNaq8lqj1DxSTZhv6ULqxy6O\n"
        "B6p657a+GgUSlAq5BOwS1O3UK6BfhrGPH3IaOuTaRwHULa5OP/mkmTJk9TDA3dp7\n"
        "fsnPI+1rZGArGO6301TdID0GwM9TsWTnRUtuSRovNYVU7bPtgxrBGOnKWqa0QzbG\n"
        "cO/9vTjzAgMBAAECggEAUbRXVonRe3fZ/iC5jsop82Bs+qCf9GpY2l42t8U4Z6Rj\n"
        "is8lzicgYj05Iy+NnVLwKdkSNeUOjIpWb4l7c8AD3Mw7ZoTK9VZVs8fy6Bir2kMS\n"
        "6qs157229uxXOINbqG9aqRMhD/LDdvyH0HTxekeTzkg9YOfXZrgjtepwTyKH3YuX\n"
        "tFAALyLw70BXD7aLKxkA+R8lCCeb6UGOGte8ufHmv5zX7rlPSzJ4UlXwyyoZ7oc8\n"
        "a2W3EAU9xfqDt3Afzlue7Gl3F0i1VhZS7L0AHz4yoPd8UzaMM7TqE44n6kBPjw4x\n"
        "0XPxdT9ixbpdbhxvyak3dTzID/BqJjtnH3ZcmInEWQKBgQDt7dzCETGjAf1odAtm\n"
        "u71WEQonhOqjQF25EmkUchO5OMPHzLMzgkEzlqN/Y4yFHnWovuHI2RvDjliBbRVX\n"
        "HOrYzhZuLJYED6U1YRwpfmNutlx0Ov5xDrSACNest019GlteVhkMwbMAdE0zYvda\n"
        "6NPAaCnTERmn0uo+wgPh6O8P9wKBgQDTXx8HrjZ3uPF1ciRIv6eosEuoYBk2SatL\n"
        "RWjse3FzkxJp+7wi4ZSftex7oBvet14doTakFPKh30WdsnMCbLSMl848jc3V7h0D\n"
        "aIv/6kgDt15V38DIfo3urmTDgx08MjNLf7cWvLuX8JnKmczRH5MQFt4m6xCk9FLh\n"
        "IUru2oFX5QKBgQDgJd98oNAfXmeKeRCopyOXKpRjaK/YPnJNQY/nKBV2wIiI+Uq1\n"
        "sveUD01WXMVsP6be4304R3kFgVvWCps5J1AEEhtFjhGCH+8nW6Y5XQ11pq1w/obo\n"
        "oc7gc/QjjcidnDAJrpkI0I6/VdHJIGohpgifUkJcytZdqbDukL8xm81kPwKBgB57\n"
        "SjNGWU58mj2Qn1Dqapg2DEMU0K32g8BQGyh3SM4c+HvWNLRnCZLanmbZmQtOMEFf\n"
        "nUyeRLs5QoXSVSmVWZgo0DIFVRizxPVX31ChjY8To4QIx3liXr974MYdI1sp9iG+\n"
        "Ij4iK39m0BbahSHnRsL8OT8tiPHw1f7nWTIGyr5NAoGBAMh1QmJ8r6ypmkhDVD4u\n"
        "65DP2F62RqV7CE0jAmzuvr4BWGmISZlrkgufZS1CFNrbmki3vxJ9jXFbf++1IZOf\n"
        "9une2dw9xRtb7DfWk7hj9juphmrFOfjlAWBppm0XLAo0sEWnsoP8jY59DQZL+Pe2\n"
        "L4A+7H0uIXotBcvK7kuK0nwY\n"
        "-----END PRIVATE KEY-----\n";

  const std::string cert
      = "-----BEGIN CERTIFICATE-----\n"
        "MIIDmDCCAoCgAwIBAgIUC/F8EaviczpG6+rQZ+QdP+RDOMEwDQYJKoZIhvcNAQEL\n"
        "BQAwSjEUMBIGA1UEAwwLZml0b3JpYS5jb20xCzAJBgNVBAYTAlRXMRQwEgYDVQQI\n"
        "DAtUYWlwZWkgQ2l0eTEPMA0GA1UEBwwGVGFpcGVpMCAXDTIyMTIyMjE3NDUyOVoY\n"
        "DzIwNTIxMjE0MTc0NTI5WjBKMRQwEgYDVQQDDAtmaXRvcmlhLmNvbTELMAkGA1UE\n"
        "BhMCVFcxFDASBgNVBAgMC1RhaXBlaSBDaXR5MQ8wDQYDVQQHDAZUYWlwZWkwggEi\n"
        "MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDEc3UP0Vcfp4pRJ4JxLAcTyutW\n"
        "gbqaLup9TOc20i18DHcL5//mRkIAaNjsYuzIW2rvrcNy7I2cK6UQjrZssu8vuNjy\n"
        "BFpfMQbsTTDK09QcNCfp96V88HZ1fQnEMlfaAlbrGcOGbV/rQyH6/eLbxA9+4Mlz\n"
        "wAYlzub4enTCJGnBboSK3X5vEWW6UNaq8lqj1DxSTZhv6ULqxy6OB6p657a+GgUS\n"
        "lAq5BOwS1O3UK6BfhrGPH3IaOuTaRwHULa5OP/mkmTJk9TDA3dp7fsnPI+1rZGAr\n"
        "GO6301TdID0GwM9TsWTnRUtuSRovNYVU7bPtgxrBGOnKWqa0QzbGcO/9vTjzAgMB\n"
        "AAGjdDByMB0GA1UdDgQWBBROHBqHOS6AtSO+m+ZPcebm0sOnEDAfBgNVHSMEGDAW\n"
        "gBROHBqHOS6AtSO+m+ZPcebm0sOnEDAOBgNVHQ8BAf8EBAMCBaAwIAYDVR0lAQH/\n"
        "BBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMA0GCSqGSIb3DQEBCwUAA4IBAQBZmFEO\n"
        "aU+sPJ1a4WOKD6zf2kqH1Y6B/K9db3b6I+GU+aF65/8npdJT5RFs8bkpZw53TJb2\n"
        "a1avH7cjx/ocSmBrQL3HuLKHaNsPU4PMMTxgiQaFgejrDgjY64QGObT6HCew2mZ9\n"
        "KTn4mOtGN+GBhxoGtX0JeRjV1lIzVr0vdXMsgVv7F7Mbb3+soRrZHEQtVYIdsI/5\n"
        "dzfE9BZmTa1qa5PiDVDUPv64EsPUgkBHSNfAx6BdUDzydCiVNglHJ+zyK+m4Md8V\n"
        "FVUg8EmC0KA6lW9IO0IYvGc2YfporprxFUe1Snkl+fGrs0wnhPdceYdWhGPCSoZ8\n"
        "dvAf5JDZXjiI6LaZ\n"
        "-----END CERTIFICATE-----\n";

  const std::string dh
      = "-----BEGIN DH PARAMETERS-----\n"
        "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
        "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
        "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
        "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
        "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
        "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
        "-----END DH PARAMETERS-----\n";

  net::ssl::context ssl_ctx(net::ssl::context::method::tlsv12_server);
  ssl_ctx.set_options(boost::asio::ssl::context::default_workarounds
                      | boost::asio::ssl::context::no_sslv2
                      | boost::asio::ssl::context::no_sslv3
                      | boost::asio::ssl::context::single_dh_use);

  ssl_ctx.use_certificate_chain(boost::asio::buffer(cert.data(), cert.size()));

  ssl_ctx.use_private_key(boost::asio::buffer(key.data(), key.size()),
                          boost::asio::ssl::context::file_format::pem);

  ssl_ctx.use_tmp_dh(boost::asio::buffer(dh.data(), dh.size()));

  return ssl_ctx;
}

net::ssl::context get_client_ssl_ctx()
{
  net::ssl::context ssl_ctx(net::ssl::context::method::tlsv12_client);
  ssl_ctx.set_verify_mode(net::ssl::verify_none);
  return ssl_ctx;
}

const char* localhost = "127.0.0.1";
const std::uint16_t port = 8080;

}

TEST_CASE("http_server_config")
{
  auto server = http_server(
      http_server_config()
          .set_threads(1)
          .set_max_listen_connections(2048)
          .set_client_request_timeout(std::chrono::seconds(1))
          .route(router(methods::get, "/get",
                        [&]([[maybe_unused]] http_context& c)
                            -> net::awaitable<void> { co_return; })));
  server.run(localhost, port);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

TEST_CASE("middlewares and router's invocation order")
{
  int state = 0;
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
          .route(methods::get, "/get",
                 [&]([[maybe_unused]] http_context& c) -> net::awaitable<void> {
                   CHECK_EQ(++state, 3);
                   co_return;
                 })));
  server.run(localhost, port);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  auto resp = simple_http_client(localhost, port)
                  .with(methods::get)
                  .with_target("/api/get")
                  .with_field(fields::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), status::ok);

  CHECK_EQ(++state, 6);
}

namespace simple_http_request_test {

void configure_server(http_server_config& config)
{
  config.route(router(
      methods::get, "/api/v1/users/:user/filmography/years/:year",
      [&](http_context& c) -> net::awaitable<void> {
        CHECK_EQ(c.path(), "/api/v1/users/:user/filmography/years/:year");
        CHECK_EQ(c.encoded_params().size(), 2);
        CHECK_EQ((*c.encoded_params().find("user")).value, R"(Rina%20Hikada)");
        CHECK_EQ((*c.encoded_params().find("year")).value, R"(2022)");
        CHECK_EQ(c.params().size(), 2);
        CHECK_EQ((*c.params().find("user")).value, "Rina Hikada");
        CHECK_EQ((*c.params().find("year")).value, "2022");
        CHECK_EQ(c.request().method(), methods::get);
        CHECK_EQ(c.request().encoded_path(),
                 R"(/api/v1/users/Rina%20Hikada/filmography/years/2022)");
        CHECK_EQ(c.request().path(),
                 "/api/v1/users/Rina Hikada/filmography/years/2022");
        CHECK_EQ(c.request().encoded_query(),
                 R"(name=Rina%20Hikada&birth=1994%2F06%2F15)");
        CHECK_EQ(c.request().query(), "name=Rina Hikada&birth=1994/06/15");
        CHECK_EQ(c.request().encoded_params().size(), 2);
        CHECK_EQ((*c.request().encoded_params().find("name")).value,
                 R"(Rina%20Hikada)");
        CHECK_EQ((*c.request().encoded_params().find("birth")).value,
                 R"(1994%2F06%2F15)");
        CHECK_EQ(c.request().params().size(), 2);
        CHECK_EQ((*c.request().params().find("name")).value, "Rina Hikada");
        CHECK_EQ((*c.request().params().find("birth")).value, "1994/06/15");
        CHECK_EQ(c.request().body(), "text");
        co_return;
      }));
}

void configure_client(simple_http_client& client)
{
  client.with(methods::get)
      .with_target(
          R"(/api/v1/users/Rina%20Hikada/filmography/years/2022?name=Rina%20Hikada&birth=1994%2F06%2F15)")
      .with_body("text")
      .with_field(fields::connection, "close");
}
}

TEST_CASE("simple request without tls")
{
  auto server = http_server(http_server_config().configure(
      simple_http_request_test::configure_server));
  server.run(localhost, port);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  auto client = simple_http_client(localhost, port);
  simple_http_request_test::configure_client(client);
  auto resp = client.send_request();
  CHECK_EQ(resp.result(), status::ok);
}

TEST_CASE("simple request with tls")
{
  auto server = http_server(http_server_config().configure(
      simple_http_request_test::configure_server));
  server.run_with_tls(localhost, port, get_server_ssl_ctx());
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  auto client = simple_http_client(localhost, port);
  simple_http_request_test::configure_client(client);
  auto resp = client.send_request(get_client_ssl_ctx());
  CHECK_EQ(resp.result(), status::ok);
}

TEST_CASE("response status only")
{
  auto server = http_server(http_server_config().route(
      router(methods::get, "/api", [](http_context& c) -> net::awaitable<void> {
        c.status(status::accepted);
        co_return;
      })));
  server.run(localhost, port);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  auto resp = simple_http_client(localhost, port)
                  .with(methods::get)
                  .with_target("/api")
                  .with_field(fields::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), status::accepted);
  CHECK_EQ(resp.at(fields::connection), "close");
  CHECK_EQ(resp.at(fields::content_length), "0");
  CHECK_EQ(resp.body(), "");
}

TEST_CASE("response with plain text")
{
  auto server = http_server(http_server_config().route(
      router(methods::get, "/api", [](http_context& c) -> net::awaitable<void> {
        c.plain_text("plain text");
        co_return;
      })));
  server.run(localhost, port);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  auto resp = simple_http_client(localhost, port)
                  .with(methods::get)
                  .with_target("/api")
                  .with_field(fields::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), status::ok);
  CHECK_EQ(resp.at(fields::content_type), "text/plain; charset=utf-8");
  CHECK_EQ(resp.body(), "plain text");
}

TEST_CASE("response with plain text")
{
  auto server = http_server(http_server_config().route(
      router(methods::get, "/api", [](http_context& c) -> net::awaitable<void> {
        c.json({
            { "obj_boolean", true },
            { "obj_number", 1234567 },
            { "obj_string", "str" },
            { "obj_array", json::array { false, 7654321, "rts" } },
        });
        co_return;
      })));
  server.run(localhost, port);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  auto resp = simple_http_client(localhost, port)
                  .with(methods::get)
                  .with_target("/api")
                  .with_field(fields::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), status::ok);
  CHECK_EQ(resp.at(fields::content_type), "application/json; charset=utf-8");
  CHECK_EQ(resp.body(),
           json::serialize(json::value({
               { "obj_boolean", true },
               { "obj_number", 1234567 },
               { "obj_string", "str" },
               { "obj_array", json::array { false, 7654321, "rts" } },
           })));
}

TEST_SUITE_END();
