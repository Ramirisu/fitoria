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
#include <fitoria/http_server.hpp>

using namespace fitoria;

using namespace fitoria::http_server_utils;

TEST_SUITE_BEGIN("http_server");

TEST_CASE("middlewares invocation order")
{
  int state = 0;
  const auto port = generate_port();
  auto server = http_server(http_server_config().route(
      router_group("/api")
          .use([&](http_request& req)
                   -> net::awaitable<expected<http_response, http_error>> {
            CHECK_EQ(++state, 1);
            auto resp = co_await req.next();
            CHECK_EQ(++state, 5);
            co_return resp;
          })
          .use([&](http_request& req)
                   -> net::awaitable<expected<http_response, http_error>> {
            CHECK_EQ(++state, 2);
            auto resp = co_await req.next();
            CHECK_EQ(++state, 4);
            co_return resp;
          })
          .route(verb::get, "/get",
                 [&]([[maybe_unused]] http_request& req)
                     -> net::awaitable<expected<http_response, http_error>> {
                   CHECK_EQ(++state, 3);
                   co_return http_response(status::ok);
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

TEST_SUITE_END();
