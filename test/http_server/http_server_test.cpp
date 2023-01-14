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

#include <fitoria/http_server.hpp>

using namespace fitoria;

using namespace http_server_utils;

TEST_SUITE_BEGIN("http_server");

TEST_CASE("builder")
{
  const auto port = generate_port();
  auto server = http_server::builder()
                    .set_threads(1)
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
                    .route(router(http::verb::get, "/api",
                                  [&]([[maybe_unused]] http_request& req)
                                      -> net::awaitable<http_response> {
                                    co_return http_response(http::status::ok);
                                  }))
                    .build();
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
}

TEST_CASE("invalid target")
{
  const auto port = generate_port();
  auto server = http_server::builder()
                    .route(router(http::verb::get, "/api/v1/users/{user}",
                                  []([[maybe_unused]] http_request& req)
                                      -> net::awaitable<http_response> {
                                    co_return http_response(http::status::ok);
                                  }))
                    .build();
  server.bind(server_ip, port).run();
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
    CHECK_EQ(resp.at(http::field::content_type), "text/plain");
    CHECK_EQ(resp.body(), "request path is not found");
  }
}

TEST_CASE("generic request")
{
  const auto port = generate_port();
  auto server
      = http_server::builder()
            .route(router(
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
                  test_route(req.route());
                  test_route(static_cast<const http_request&>(req).route());

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
                           "text/plain");
                  CHECK_EQ(static_cast<const http_request&>(req).headers().at(
                               http::field::content_type),
                           "text/plain");
                  CHECK_EQ(req.body(), "happy birthday");
                  CHECK_EQ(static_cast<const http_request&>(req).body(),
                           "happy birthday");

                  co_return http_response(http::status::ok);
                }))
            .build();
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp
      = simple_http_client(localhost, port)
            .with_target(
                R"(/api/v1/users/Rina%20Hidaka/filmography/years/2022?name=Rina%20Hidaka&birth=1994%2F06%2F15)")
            .with(http::verb::get)
            .with_field(http::field::content_type, "text/plain")
            .with_field(http::field::connection, "close")
            .with_body("happy birthday")
            .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
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
            .route(router(http::verb::get, "/api",
                          []([[maybe_unused]] http_request& req)
                              -> net::awaitable<http_response> {
                            co_return http_response(http::status::accepted);
                          }))
            .build();
  server.bind(server_ip, port).run();
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
}

TEST_CASE("response with plain text")
{
  const auto port = generate_port();
  auto server = http_server::builder()
                    .route(router(http::verb::get, "/api",
                                  []([[maybe_unused]] http_request& req)
                                      -> net::awaitable<http_response> {
                                    co_return http_response(http::status::ok)
                                        .set_header(http::field::content_type,
                                                    "text/plain")
                                        .set_body("plain text");
                                  }))
                    .build();
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api")
                  .with_field(http::field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
  CHECK_EQ(resp.at(http::field::content_type), "text/plain");
  CHECK_EQ(resp.body(), "plain text");
}

TEST_CASE("response with json")
{
  const auto port = generate_port();
  auto server
      = http_server::builder()
            .route(router(http::verb::get, "/api",
                          []([[maybe_unused]] http_request& req)
                              -> net::awaitable<http_response> {
                            co_return http_response(http::status::ok)
                                .set_header(http::field::content_type,
                                            "application/json")
                                .set_json({
                                    { "obj_boolean", true },
                                    { "obj_number", 1234567 },
                                    { "obj_string", "str" },
                                    { "obj_array",
                                      json::array { false, 7654321, "rts" } },
                                });
                          }))
            .build();
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api")
                  .with_field(http::field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
  CHECK_EQ(resp.at(http::field::content_type), "application/json");
  CHECK_EQ(resp.body(),
           json::serialize(json::value({
               { "obj_boolean", true },
               { "obj_number", 1234567 },
               { "obj_string", "str" },
               { "obj_array", json::array { false, 7654321, "rts" } },
           })));
}

TEST_CASE("response with struct to json")
{
  const auto port = generate_port();
  auto server = http_server::builder()
                    .route(router(http::verb::get, "/api",
                                  []([[maybe_unused]] http_request& req)
                                      -> net::awaitable<http_response> {
                                    co_return http_response(http::status::ok)
                                        .set_header(http::field::content_type,
                                                    "application/json")
                                        .set_json(user_t {
                                            .name = "Rina Hidaka",
                                            .birth = "1994/06/15",
                                        });
                                  }))
                    .build();
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api")
                  .with_field(http::field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
  CHECK_EQ(resp.at(http::field::content_type), "application/json");
  CHECK_EQ(json::value_to<user_t>(json::parse(resp.body())),
           user_t {
               .name = "Rina Hidaka",
               .birth = "1994/06/15",
           });
}

TEST_SUITE_END();
