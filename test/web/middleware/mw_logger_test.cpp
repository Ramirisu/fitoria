//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria_http_server_utils.h>
#include <fitoria_simple_http_client.h>

#include <fitoria/web.hpp>

using namespace fitoria;

using namespace fitoria::http_server_utils;

TEST_SUITE_BEGIN("http_server.middleware.logger");

TEST_CASE("logger middleware")
{
  // TODO: verify logging content

  const auto port = generate_port();
  auto server
      = http_server::builder()
            .route(scope("/api")
                       .use(middleware::logger())
                       .route(http::verb::get, "/get",
                              [&]([[maybe_unused]] http_request& req)
                                  -> net::awaitable<http_response> {
                                co_return http_response(http::status::ok);
                              }))
            .build();
  server.bind(server_ip, port).run();
  std::this_thread::sleep_for(server_start_wait_time);

  auto resp = simple_http_client(localhost, port)
                  .with(http::verb::get)
                  .with_target("/api/get")
                  .with_field(http::field::user_agent, "fitoria")
                  .with_field(http::field::connection, "close")
                  .send_request();
  CHECK_EQ(resp.result(), http::status::ok);
}

TEST_SUITE_END();
