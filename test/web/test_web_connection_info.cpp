//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.connection_info]");

TEST_CASE("connection_info")
{
  auto server = http_server::builder()
                    .serve(route::get<"/">(
                        [](http_request& req) -> net::awaitable<http_response> {
                          CHECK_EQ(req.connection().local_addr(),
                                   net::ip::make_address("127.0.0.1"));
                          CHECK_EQ(req.connection().local_port(), 0);
                          CHECK_EQ(req.connection().remote_addr(),
                                   net::ip::make_address("127.0.0.1"));
                          CHECK_EQ(req.connection().remote_port(), 0);
                          CHECK_EQ(req.connection().listen_addr(),
                                   net::ip::make_address("127.0.0.1"));
                          CHECK_EQ(req.connection().listen_port(), 0);
                          co_return http_response(http::status::ok);
                        }))
                    .build();

  net::sync_wait([&]() -> net::awaitable<void> {
    auto res = co_await server.async_serve_request(
        "/", http_request(http::verb::get));
    CHECK_EQ(res.status_code(), http::status::ok);
  }());
}

TEST_SUITE_END();
