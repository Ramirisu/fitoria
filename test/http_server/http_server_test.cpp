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

#include <fitoria/http_server.hpp>

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
          .route(router(
              verb::get, "/api",
              [&]([[maybe_unused]] http_request& req)
                  -> net::awaitable<expected<http_response, http_error>> {
                co_return http_response(status::ok);
              })));
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(verb::get)
                  .with_target("/api")
                  .send_request();
  CHECK_EQ(resp.result(), status::ok);
}

TEST_SUITE_END();
