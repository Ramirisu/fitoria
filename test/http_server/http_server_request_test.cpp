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

TEST_SUITE_BEGIN("http_server.request");

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

TEST_SUITE_END();
