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

using namespace fitoria;

using namespace http_server_utils;

TEST_SUITE_BEGIN("web.http_server");

TEST_CASE("builder")
{
  const auto port = generate_port();
  auto server = http_server::builder()
                    .set_max_listen_connections(2048)
                    .set_client_request_timeout(std::chrono::seconds(1))
                    .set_exception_handler([](std::exception_ptr ptr) {
                      if (!ptr) {
                        return;
                      }
                      try {
                        std::rethrow_exception(ptr);
                      } catch (...) {
                      }
                    })
                    .route(route(http::verb::get, "/api",
                                 [&]([[maybe_unused]] http_request& req)
                                     -> net::awaitable<http_response> {
                                   co_return http_response(http::status::ok);
                                 }))
                    .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> net::awaitable<void> { co_await server.async_run(); },
      net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
  ioc.stop();
}

TEST_CASE("duplicate route")
{
  CHECK_THROWS_AS(http_server::builder().route(
                      scope("/api/v1")
                          .route(http::verb::get, "/xxx",
                                 []([[maybe_unused]] http_request& req)
                                     -> net::awaitable<http_response> {
                                   co_return http_response(http::status::ok);
                                 })
                          .route(http::verb::get, "/xxx",
                                 []([[maybe_unused]] http_request& req)
                                     -> net::awaitable<http_response> {
                                   co_return http_response(http::status::ok);
                                 })),
                  std::system_error);
}

TEST_CASE("invalid target")
{
  const auto port = generate_port();
  auto server = http_server::builder()
                    .route(route(http::verb::get, "/api/v1/users/{user}",
                                 []([[maybe_unused]] http_request& req)
                                     -> net::awaitable<http_response> {
                                   co_return http_response(http::status::ok);
                                 }))
                    .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> net::awaitable<void> { co_await server.async_run(); },
      net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  std::this_thread::sleep_for(server_start_wait_time);

  const auto test_cases = std::vector {
    "/", "/a", "/api", "/api/", "/api/v1", "/api/v1/x", "/api/v1/users/x/y",
  };

  for (auto& test_case : test_cases) {
    auto resp = simple_http_client(localhost, port)
                    .with_target(test_case)
                    .with(http::verb::get)
                    .with_field(http::field::connection, "close")
                    .send_request();
    CHECK_EQ(resp.result(), http::status::not_found);
    CHECK_EQ(resp.at(http::field::content_type),
             http::fields::content_type::plaintext());
    CHECK_EQ(resp.body(), "request path is not found");
  }
  ioc.stop();
}

TEST_CASE("expect: 100-continue")
{
  const auto port = generate_port();
  auto server = http_server::builder()
                    .route(route(http::verb::post, "/api/v1/post",
                                 []([[maybe_unused]] http_request& req)
                                     -> net::awaitable<http_response> {
                                   CHECK_EQ(req.body(), "text");
                                   co_return http_response(http::status::ok);
                                 }))
                    .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> net::awaitable<void> { co_await server.async_run(); },
      net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with_target("/api/v1/post")
                  .with(http::verb::post)
                  .with_field(http::field::expect, "100-continue")
                  .with_field(http::field::connection, "close")
                  .with_body("text")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
  ioc.stop();
}

TEST_CASE("unhandled exception from handler")
{
  bool got_exception = false;
  const auto port = generate_port();
  auto server = http_server::builder()
                    .set_exception_handler([&](std::exception_ptr ptr) {
                      if (ptr) {
                        try {
                          std::rethrow_exception(ptr);
                        } catch (const std::exception&) {
                          got_exception = true;
                        }
                      }
                    })
                    .route(route(http::verb::get, "/api/v1/get",
                                 []([[maybe_unused]] http_request& req)
                                     -> net::awaitable<http_response> {
                                   throw std::exception();
                                   co_return http_response(http::status::ok);
                                 }))
                    .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> net::awaitable<void> { co_await server.async_run(); },
      net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  std::this_thread::sleep_for(server_start_wait_time);

  CHECK_THROWS(simple_http_client(localhost, port)
                   .with_target("/api/v1/get")
                   .with(http::verb::get)
                   .with_field(http::field::connection, "close")
                   .send_request());
  CHECK(got_exception);
  ioc.stop();
}

TEST_CASE("middlewares invocation order")
{
  int state = 0;
  auto server
      = http_server::builder()
            .route(
                scope("/api")
                    .use([&](http_context& c) -> net::awaitable<http_response> {
                      CHECK_EQ(++state, 1);
                      auto resp = co_await c.next();
                      CHECK_EQ(++state, 5);
                      co_return resp;
                    })
                    .use([&](http_context& c) -> net::awaitable<http_response> {
                      CHECK_EQ(++state, 2);
                      auto resp = co_await c.next();
                      CHECK_EQ(++state, 4);
                      co_return resp;
                    })
                    .route(http::verb::get, "/get",
                           [&]([[maybe_unused]] http_request& req)
                               -> net::awaitable<http_response> {
                             CHECK_EQ(++state, 3);
                             co_return http_response(http::status::ok);
                           }))
            .build();

  auto res = server.serve_http_request(
      "/api/get", http_request().set_method(http::verb::get));
  CHECK_EQ(res.status_code(), http::status::ok);
  CHECK_EQ(++state, 6);
}

TEST_CASE("generic request")
{
  const auto port = generate_port();
  auto server
      = http_server::builder()
            .route(route(
                http::verb::get,
                "/api/v1/users/{user}/filmography/years/{year}",
                [](http_request& req) -> net::awaitable<http_response> {
                  CHECK_EQ(req.local_endpoint().address(),
                           net::ip::make_address(server_ip));
                  CHECK_EQ(req.remote_endpoint().address(),
                           net::ip::make_address(server_ip));

                  auto test_route = [](auto& route) {
                    CHECK_EQ(route.path(),
                             "/api/v1/users/{user}/filmography/years/{year}");

                    CHECK_EQ(route.at("user"), "Rina Hidaka");
                    CHECK_EQ(route.at("year"), "2022");
                  };
                  test_route(req.route_params());

                  CHECK_EQ(req.method(), http::verb::get);
                  CHECK_EQ(req.path(),
                           "/api/v1/users/Rina Hidaka/filmography/years/2022");

                  auto test_query = [](auto& query) {
                    CHECK_EQ(query.size(), 2);
                    CHECK_EQ(query.at("name"), "Rina Hidaka");
                    CHECK_EQ(query.at("birth"), "1994/06/15");
                  };
                  test_query(req.query());
                  test_query(static_cast<const http_request&>(req).query());

                  CHECK_EQ(req.headers().at(http::field::content_type),
                           http::fields::content_type::plaintext());
                  CHECK_EQ(static_cast<const http_request&>(req).headers().at(
                               http::field::content_type),
                           http::fields::content_type::plaintext());
                  CHECK_EQ(req.body(), "happy birthday");
                  CHECK_EQ(static_cast<const http_request&>(req).body(),
                           "happy birthday");

                  co_return http_response(http::status::ok);
                }))
            .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> net::awaitable<void> { co_await server.async_run(); },
      net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp
      = simple_http_client(localhost, port)
            .with_target(
                R"(/api/v1/users/Rina%20Hidaka/filmography/years/2022?name=Rina%20Hidaka&birth=1994%2F06%2F15)")
            .with(http::verb::get)
            .with_field(http::field::content_type,
                        http::fields::content_type::plaintext())
            .with_field(http::field::connection, "close")
            .with_body("happy birthday")
            .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
  ioc.stop();
}

namespace {

struct user_t {
  std::string name;
  std::string birth;

  friend bool operator==(const user_t&, const user_t&) = default;
};

user_t tag_invoke(const json::value_to_tag<user_t>&, const json::value& jv)
{
  return user_t {
    .name = std::string(jv.at("name").as_string()),
    .birth = std::string(jv.at("birth").as_string()),
  };
}

void tag_invoke(const json::value_from_tag&,
                json::value& jv,
                const user_t& user)
{
  jv = { { "name", user.name }, { "birth", user.birth } };
}

}

TEST_CASE("response status only")
{
  const auto port = generate_port();
  auto server
      = http_server::builder()
            .route(route(http::verb::get, "/api",
                         []([[maybe_unused]] http_request& req)
                             -> net::awaitable<http_response> {
                           co_return http_response(http::status::accepted);
                         }))
            .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> net::awaitable<void> { co_await server.async_run(); },
      net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api")
                  .with_field(http::field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::accepted);
  CHECK_EQ(resp.at(http::field::connection), "close");
  CHECK_EQ(resp.at(http::field::content_length), "0");
  CHECK_EQ(resp.body(), "");
  ioc.stop();
}

TEST_CASE("response with plain text")
{
  const auto port = generate_port();
  auto server
      = http_server::builder()
            .route(route(http::verb::get, "/api",
                         []([[maybe_unused]] http_request& req)
                             -> net::awaitable<http_response> {
                           co_return http_response(http::status::ok)
                               .set_header(
                                   http::field::content_type,
                                   http::fields::content_type::plaintext())
                               .set_body("plain text");
                         }))
            .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> net::awaitable<void> { co_await server.async_run(); },
      net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api")
                  .with_field(http::field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
  CHECK_EQ(resp.at(http::field::content_type),
           http::fields::content_type::plaintext());
  CHECK_EQ(resp.body(), "plain text");
  ioc.stop();
}

TEST_CASE("response with json")
{
  const auto port = generate_port();
  auto server
      = http_server::builder()
            .route(route(http::verb::get, "/api",
                         []([[maybe_unused]] http_request& req)
                             -> net::awaitable<http_response> {
                           co_return http_response(http::status::ok)
                               .set_header(http::field::content_type,
                                           http::fields::content_type::json())
                               .set_json({
                                   { "obj_boolean", true },
                                   { "obj_number", 1234567 },
                                   { "obj_string", "str" },
                                   { "obj_array",
                                     json::array { false, 7654321, "rts" } },
                               });
                         }))
            .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> net::awaitable<void> { co_await server.async_run(); },
      net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api")
                  .with_field(http::field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
  CHECK_EQ(resp.at(http::field::content_type),
           http::fields::content_type::json());
  CHECK_EQ(resp.body(),
           json::serialize(json::value({
               { "obj_boolean", true },
               { "obj_number", 1234567 },
               { "obj_string", "str" },
               { "obj_array", json::array { false, 7654321, "rts" } },
           })));
  ioc.stop();
}

TEST_CASE("response with struct to json")
{
  const auto port = generate_port();
  auto server
      = http_server::builder()
            .route(route(http::verb::get, "/api",
                         []([[maybe_unused]] http_request& req)
                             -> net::awaitable<http_response> {
                           co_return http_response(http::status::ok)
                               .set_header(http::field::content_type,
                                           http::fields::content_type::json())
                               .set_json(user_t {
                                   .name = "Rina Hidaka",
                                   .birth = "1994/06/15",
                               });
                         }))
            .build();
  server.bind(server_ip, port);
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> net::awaitable<void> { co_await server.async_run(); },
      net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api")
                  .with_field(http::field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
  CHECK_EQ(resp.at(http::field::content_type),
           http::fields::content_type::json());
  CHECK_EQ(json::value_to<user_t>(json::parse(resp.body())),
           user_t {
               .name = "Rina Hidaka",
               .birth = "1994/06/15",
           });
  ioc.stop();
}

TEST_SUITE_END();
