//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#define FITORIA_USE_IO_CONTEXT_EXECUTOR
#include <fitoria/test/test.hpp>

#include <fitoria/test/async_readable_chunk_stream.hpp>
#include <fitoria/test/http_client.hpp>
#include <fitoria/test/http_server_utils.hpp>
#include <fitoria/test/utility.hpp>

#include <fitoria/web.hpp>

#include <boost/scope/scope_exit.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.http_server.io_context]");

TEST_CASE("compile with io_context")
{
  const auto port = generate_port();
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::post<"/">(
                        [](std::string str)
                            -> net::awaitable<response,
                                              net::io_context::executor_type> {
                          CHECK_EQ(str, "");
                          co_return response::ok().build();
                        }))
                    .build();
  REQUIRE(server.bind(localhost, port));

  auto worker = std::thread([&]() { ioc.run(); });
  auto guard = boost::scope::make_scope_exit([&]() {
    ioc.stop();
    worker.join();
  });
  std::this_thread::sleep_for(server_start_wait_time);

  net::co_spawn(
      ioc,
      [&]() -> net::awaitable<void, net::io_context::executor_type> {
        auto res
            = co_await http_client()
                  .set_method(http::verb::post)
                  .set_url(to_local_url(boost::urls::scheme::http, port, "/"))
                  .set_header(http::field::connection, "close")
                  .set_plaintext("")
                  .async_send();
        CHECK_EQ(res->status_code(), http::status::ok);
      },
      net::use_future)
      .get();
}

TEST_SUITE_END();
