//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#define FITORIA_USE_IO_CONTEXT_EXECUTOR
#include <fitoria/test/test.hpp>

#include <fitoria/test/http_client.hpp>
#include <fitoria/test/http_server_utils.hpp>

using namespace fitoria;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.client.http_client.io_context]");

TEST_CASE("compile with io_context")
{
  sync_wait([]() -> net::awaitable<void, net::io_context::executor_type> {
    auto res = co_await http_client()
                   .set_method(http::verb::post)
                   .set_url("http://httpbun.com/post")
                   .set_body("echo")
                   .async_send();
    CHECK_EQ(res->status().value(), http::status::ok);
    CHECK_EQ((co_await res->as_json())->at("data"), "echo");
  });
}

TEST_SUITE_END();
