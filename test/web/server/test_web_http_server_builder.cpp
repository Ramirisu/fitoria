//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/client.hpp>
#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;
using fitoria::client::http_client;

TEST_SUITE_BEGIN("[fitoria.web.http_server.builder]");

TEST_CASE("builder")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .set_max_listen_connections(2048)
                    .set_tls_handshake_timeout(std::chrono::seconds(5))
                    .set_request_timeout(std::chrono::seconds(10))
                    .set_request_header_limit(nullopt)
                    .set_request_body_limit(nullopt)
#if !FITORIA_NO_EXCEPTIONS
                    .set_exception_handler([](std::exception_ptr ptr) {
                      if (ptr) {
                        try {
                          std::rethrow_exception(ptr);
                        } catch (...) {
                          REQUIRE(false);
                        }
                      }
                    })
#endif
                    .serve(route::get<"/">([&]() -> awaitable<response> {
                      co_return response::ok().build();
                    }))
                    .build();
  REQUIRE(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  REQUIRE_EQ(server.max_listen_connections(), 2048);
  REQUIRE_EQ(server.tls_handshake_timeout(), std::chrono::seconds(5));
  REQUIRE_EQ(server.request_timeout(), std::chrono::seconds(10));
  REQUIRE_EQ(server.request_header_limit(), nullopt);
  REQUIRE_EQ(server.request_body_limit(), nullopt);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::get)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_header(http::field::connection, "close")
                  .async_send();
        REQUIRE_EQ(res->status_code(), http::status::ok);
        REQUIRE_EQ(co_await res->as_string(), "");
      },
      net::use_future)
      .get();
}

TEST_SUITE_END();
