//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.request]");

template <typename Builder>
auto test_builder(Builder&& builder)
{
  auto req = std::forward<Builder>(builder)
                 .set_method(http::verb::connect)
                 .set_header(http::field::content_type, "text/plain")
                 .set_header("Content-Encoding", "deflate")
                 .insert_header(http::field::accept_encoding, "gzip")
                 .insert_header("Range", "bytes=0-99")
                 .set_query("key", "value")
                 .build();
  CHECK_EQ(req.method(), http::verb::connect);
  CHECK_EQ(req.headers().get("Content-Type"), "text/plain");
  CHECK_EQ(req.headers().get(http::field::content_encoding), "deflate");
  CHECK_EQ(req.headers().get("Accept-Encoding"), "gzip");
  CHECK_EQ(req.headers().get(http::field::range), "bytes=0-99");
  CHECK_EQ(req.query().get("key"), "value");
  return req;
}

TEST_CASE("builder with lvalue")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">([](request& req) -> awaitable<response> {
              auto builder = req.builder();
              req = test_builder(builder);
              co_return response::ok().build();
            }))
            .build();

  server.serve_request("/",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });

  ioc.run();
}

TEST_CASE("builder with rvalue")
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">([](request& req) -> awaitable<response> {
              req = test_builder(req.builder());
              co_return response::ok().build();
            }))
            .build();

  server.serve_request("/",
                       test_request::get().build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::ok);
                         co_return;
                       });

  ioc.run();
}

TEST_SUITE_END();
