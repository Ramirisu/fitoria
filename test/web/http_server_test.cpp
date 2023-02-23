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

TEST_SUITE_BEGIN("web.http_server");

TEST_CASE("builder")
{
  const auto port = generate_port();
  auto server
      = http_server::builder()
            .set_max_listen_connections(2048)
            .set_client_request_timeout(std::chrono::seconds(1))
            .set_network_error_handler(
                []([[maybe_unused]] net::error_code ec) {})
#if !FITORIA_NO_EXCEPTIONS
            .set_exception_handler([](std::exception_ptr ptr) {
              if (!ptr) {
                return;
              }
              try {
                std::rethrow_exception(ptr);
              } catch (...) {
              }
            })
#endif
            .route(route::GET<"/api">(
                [&]([[maybe_unused]] http_request& req) -> lazy<http_response> {
                  co_return http_response(http::status::ok);
                }))
            .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> lazy<void> { co_await server.async_run(); }, net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto res
      = http_client::GET(to_local_url(boost::urls::scheme::http, port, "/api"))
            .send()
            .value();
  CHECK_EQ(res.status_code(), http::status::ok);
}

TEST_CASE("duplicate route")
{
  CHECK_THROWS_AS(http_server::builder().route(
                      scope<"/api/v1">()
                          .GET<"/xxx">([]([[maybe_unused]] http_request& req)
                                           -> lazy<http_response> {
                            co_return http_response(http::status::ok);
                          })
                          .GET<"/xxx">([]([[maybe_unused]] http_request& req)
                                           -> lazy<http_response> {
                            co_return http_response(http::status::ok);
                          })),
                  std::system_error);
}

TEST_CASE("invalid target")
{
  const auto port = generate_port();
  auto server
      = http_server::builder()
            .route(route::GET<"/api/v1/users/{user}">(
                []([[maybe_unused]] http_request& req) -> lazy<http_response> {
                  co_return http_response(http::status::ok);
                }))
            .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> lazy<void> { co_await server.async_run(); }, net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  const auto test_cases = std::vector {
    "/", "/a", "/api", "/api/", "/api/v1", "/api/v1/x", "/api/v1/users/x/y",
  };

  for (auto& test_case : test_cases) {
    auto res = http_client::GET(
                   to_local_url(boost::urls::scheme::http, port, test_case))
                   .set_field(http::field::connection, "close")
                   .set_plaintext("text")
                   .send()
                   .value();
    CHECK_EQ(res.status_code(), http::status::not_found);
    CHECK_EQ(res.fields().get(http::field::content_type),
             http::fields::content_type::plaintext());
    CHECK_EQ(res.body(), "request path is not found");
  }
}

TEST_CASE("expect: 100-continue")
{
  const auto port = generate_port();
  auto server = http_server::builder()
                    .route(route::POST<"/api/v1/post">(
                        [](std::string body) -> lazy<http_response> {
                          CHECK_EQ(body, "text");
                          co_return http_response(http::status::ok);
                        }))
                    .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> lazy<void> { co_await server.async_run(); }, net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto res = http_client::POST(
                 to_local_url(boost::urls::scheme::http, port, "/api/v1/post"))
                 .set_field(http::field::expect, "100-continue")
                 .set_field(http::field::connection, "close")
                 .set_plaintext("text")
                 .send()
                 .value();
  CHECK_EQ(res.status_code(), http::status::ok);
}

#if !FITORIA_NO_EXCEPTIONS

TEST_CASE("unhandled exception from handler")
{
  bool got_exception = false;
  const auto port = generate_port();
  auto server
      = http_server::builder()
            .set_exception_handler([&](std::exception_ptr ptr) {
              if (ptr) {
                try {
                  std::rethrow_exception(ptr);
                } catch (const std::exception&) {
                  got_exception = true;
                }
              }
            })
            .route(route::GET<"/api/v1/get">(
                []([[maybe_unused]] http_request& req) -> lazy<http_response> {
                  throw std::exception();
                  co_return http_response(http::status::ok);
                }))
            .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> lazy<void> { co_await server.async_run(); }, net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  CHECK(!http_client::GET(
             to_local_url(boost::urls::scheme::http, port, "/api/v1/get"))
             .set_field(http::field::connection, "close")
             .set_plaintext("text")
             .send());

  // wait for exception thrown
  std::this_thread::sleep_for(std::chrono::seconds(1));
  CHECK(got_exception);
}

#endif

TEST_CASE("generic request")
{
  const auto port = generate_port();
  auto server
      = http_server::builder()
            .route(route::GET<"/api/v1/users/{user}/filmography/years/{year}">(
                [=](http_request& req, const connection_info& conn_info,
                    route_params& params, query_map& query, http_fields& fields,
                    std::string body) -> lazy<http_response> {
                  auto test_conn_info = [=](auto& conn_info) {
                    CHECK_EQ(conn_info.local_addr(),
                             net::ip::make_address(server_ip));
                    CHECK_EQ(conn_info.remote_addr(),
                             net::ip::make_address(server_ip));
                    CHECK_EQ(conn_info.listen_addr(),
                             net::ip::make_address(server_ip));
                    CHECK_EQ(conn_info.listen_port(), port);
                  };
                  test_conn_info(req.conn_info());
                  test_conn_info(conn_info);

                  CHECK_EQ(req.method(), http::verb::get);
                  CHECK_EQ(req.path(),
                           "/api/v1/users/Rina Hidaka/filmography/years/2022");

                  auto test_params = [](auto& params) {
                    CHECK_EQ(params.path(),
                             "/api/v1/users/{user}/filmography/years/{year}");

                    CHECK_EQ(params.at("user"), "Rina Hidaka");
                    CHECK_EQ(params.at("year"), "2022");
                  };
                  test_params(req.params());
                  test_params(params);

                  auto test_query = [](auto& query) {
                    CHECK_EQ(query.size(), 2);
                    CHECK_EQ(query.at("name"), "Rina Hidaka");
                    CHECK_EQ(query.at("birth"), "1994/06/15");
                  };
                  test_query(req.query());
                  test_query(static_cast<const http_request&>(req).query());
                  test_query(query);

                  auto test_fields = [](auto& fields) {
                    CHECK_EQ(fields.get(http::field::content_type),
                             http::fields::content_type::plaintext());
                  };

                  test_fields(req.fields());
                  test_fields(static_cast<const http_request&>(req).fields());
                  test_fields(fields);

                  CHECK(range_in_set(
                      req.fields().equal_range(http::field::user_agent),
                      [](auto&& p) { return p->second; },
                      std::set<std::string_view> { BOOST_BEAST_VERSION_STRING,
                                                   "fitoria" }));

                  CHECK_EQ(body, "happy birthday");
                  CHECK(!(co_await req.body().async_read_next()));
                  CHECK(!(co_await async_read_all<std::string>(req.body())));

                  co_return http_response(http::status::ok)
                      .insert_field(http::field::user_agent,
                                    BOOST_BEAST_VERSION_STRING)
                      .insert_field(http::field::user_agent, "fitoria");
                }))
            .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> lazy<void> { co_await server.async_run(); }, net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto res
      = http_client::GET(
            to_local_url(boost::urls::scheme::http, port,
                         "/api/v1/users/Rina Hidaka/filmography/years/2022"))
            .set_query("name", "Rina Hidaka")
            .set_query("birth", "1994/06/15")
            .set_field(http::field::connection, "close")
            .insert_field(http::field::user_agent, BOOST_BEAST_VERSION_STRING)
            .insert_field(http::field::user_agent, "fitoria")
            .set_plaintext("happy birthday")
            .send()
            .value();
  CHECK_EQ(res.status_code(), http::status::ok);
  CHECK(range_in_set(
      res.fields().equal_range(http::field::user_agent),
      [](auto&& p) { return p->second; },
      std::set<std::string_view> { BOOST_BEAST_VERSION_STRING, "fitoria" }));
}

namespace {
template <std::size_t ChunkSize>
class test_async_readable_chunk_stream {
public:
  test_async_readable_chunk_stream(std::string_view data)
      : data_(data)
  {
  }

  auto is_chunked() const noexcept
  {
    return true;
  }

  auto async_read_next()
      -> lazy<optional<expected<std::vector<std::byte>, net::error_code>>>
  {
    if (written >= data_.size()) {
      co_return nullopt;
    }

    const auto chunk_size = std::min(ChunkSize, data_.size() - written);
    auto sv = data_.substr(written, chunk_size);
    written += chunk_size;
    co_return to_bytes(sv);
  }

private:
  std::string_view data_;
  std::size_t written = 0;
};
}

TEST_CASE("request with chunked transfer-encoding")
{
  const auto input = std::string_view("abcdefghijklmnopqrstuvwxyz");

  const auto port = generate_port();
  auto server = http_server::builder()
                    .route(route::POST<"/post">(
                        [&](std::string data) -> lazy<http_response> {
                          CHECK_EQ(data, input);
                          co_return http_response(http::status::ok);
                        }))
                    .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> lazy<void> { co_await server.async_run(); }, net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto res
      = http_client::POST(
            to_local_url(boost::urls::scheme::http, port, "/post"))
            .set_field(http::field::connection, "close")
            .set_readable_stream(test_async_readable_chunk_stream<5>(input))
            .send()
            .value();
  CHECK_EQ(res.status_code(), http::status::ok);
}

TEST_CASE("response status only")
{
  const auto port = generate_port();
  auto server
      = http_server::builder()
            .route(route::GET<"/api">(
                []([[maybe_unused]] http_request& req) -> lazy<http_response> {
                  co_return http_response(http::status::accepted);
                }))
            .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> lazy<void> { co_await server.async_run(); }, net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto res
      = http_client::GET(to_local_url(boost::urls::scheme::http, port, "/api"))
            .set_field(http::field::connection, "close")
            .send()
            .value();
  CHECK_EQ(res.status_code(), http::status::accepted);
  CHECK_EQ(res.fields().get(http::field::connection), "close");
  CHECK_EQ(res.fields().get(http::field::content_length), "0");
  CHECK_EQ(res.body(), "");
}

TEST_CASE("response with plain text")
{
  const auto port = generate_port();
  auto server
      = http_server::builder()
            .route(route::GET<"/api">(
                []([[maybe_unused]] http_request& req) -> lazy<http_response> {
                  co_return http_response(http::status::ok)
                      .set_field(http::field::content_type,
                                 http::fields::content_type::plaintext())
                      .set_body("plain text");
                }))
            .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> lazy<void> { co_await server.async_run(); }, net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto res
      = http_client::GET(to_local_url(boost::urls::scheme::http, port, "/api"))
            .set_field(http::field::connection, "close")
            .send()
            .value();
  CHECK_EQ(res.status_code(), http::status::ok);
  CHECK_EQ(res.fields().get(http::field::connection), "close");
  CHECK_EQ(res.fields().get(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(res.body(), "plain text");
}

TEST_SUITE_END();
