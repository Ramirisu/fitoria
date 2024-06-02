//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/http_client.hpp>
#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.http_server.exception_handler]");

#if !FITORIA_NO_EXCEPTIONS

TEST_CASE("unhandled exception from handler")
{
  bool got_exception = false;
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .set_exception_handler([&](std::exception_ptr ptr) {
                      if (ptr) {
                        try {
                          std::rethrow_exception(ptr);
                        } catch (const std::exception&) {
                          got_exception = true;
                        }
                      }
                    })
                    .serve(route::get<"/">([]() -> awaitable<response> {
                      throw std::exception();
                      co_return response::ok().build();
                    }))
                    .build();
  REQUIRE(server.bind(server_ip, port));

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  scope_exit guard([&]() { ioc.stop(); });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> awaitable<void> {
        REQUIRE(
            !(co_await http_client()
                  .set_method(http::verb::get)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_header(http::field::connection, "close")
                  .set_plaintext("text")
                  .async_send()));
      },
      net::use_future)
      .get();

  // wait for exception thrown
  std::this_thread::sleep_for(std::chrono::seconds(1));
  REQUIRE(got_exception);
}

#endif

TEST_SUITE_END();
