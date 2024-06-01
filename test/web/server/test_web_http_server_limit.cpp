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

TEST_SUITE_BEGIN("[fitoria.web.http_server.limit]");

TEST_CASE("header_limit")
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .set_request_header_limit(16)
                    .serve(route::get<"/">([]() -> awaitable<response> {
                      co_return response::ok().build();
                    }))
                    .build();

  server.serve_request("/",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         REQUIRE_EQ(res.status_code(),
                                    http::status::bad_request);
                         REQUIRE_EQ(res.header().get(http::field::content_type),
                                    mime::text_plain());
                         REQUIRE_EQ(co_await res.as_string(),
                                    "request header size exceeds limit");
                         co_return;
                       });

  ioc.run();
}

TEST_SUITE_END();
