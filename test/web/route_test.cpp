//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/route.hpp>

using namespace fitoria::web;

TEST_SUITE_BEGIN("web.route");

TEST_CASE("basic")
{
  const auto r1
      = route::handle(http::verb::get, "/v1", [](int) -> int { return 0; });
  CHECK_EQ(r1.method(), http::verb::get);
  CHECK_EQ(r1.matcher().pattern(), "/v1");

  auto r2 = r1.rebind_parent("/api", {}, {});
  CHECK_EQ(r2.method(), http::verb::get);
  CHECK_EQ(r2.matcher().pattern(), "/api/v1");
}

TEST_CASE("methods")
{
  CHECK_EQ(route::GET("/", [](int) -> int { return 0; }).method(),
           http::verb::get);
  CHECK_EQ(route::POST("/", [](int) -> int { return 0; }).method(),
           http::verb::post);
  CHECK_EQ(route::PUT("/", [](int) -> int { return 0; }).method(),
           http::verb::put);
  CHECK_EQ(route::PATCH("/", [](int) -> int { return 0; }).method(),
           http::verb::patch);
  CHECK_EQ(route::DELETE_("/", [](int) -> int { return 0; }).method(),
           http::verb::delete_);
  CHECK_EQ(route::HEAD("/", [](int) -> int { return 0; }).method(),
           http::verb::head);
  CHECK_EQ(route::OPTIONS("/", [](int) -> int { return 0; }).method(),
           http::verb::options);
}

TEST_SUITE_END();
