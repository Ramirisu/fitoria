//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/basic_handler.hpp>
#include <fitoria/web/basic_middleware.hpp>
#include <fitoria/web/basic_route.hpp>

using namespace fitoria::web;

TEST_SUITE_BEGIN("web.basic_route");

namespace {

using middleware_type = basic_middleware<int, int>;
using handler_type = basic_handler<int, int>;
using route_type = basic_route<middleware_type, handler_type>;

}

TEST_CASE("basic")
{
  const auto route
      = route_type(http::verb::get, "/v1", [](int) -> int { return 0; });
  CHECK_EQ(route.method(), http::verb::get);
  CHECK_EQ(route.path(), "/v1");

  auto new_route = route.rebind_parent("/api", {}, {});
  CHECK_EQ(new_route.method(), http::verb::get);
  CHECK_EQ(new_route.path(), "/api/v1");
}

TEST_CASE("methods")
{
  CHECK_EQ(route_type::GET("/", [](int) -> int { return 0; }).method(),
           http::verb::get);
  CHECK_EQ(route_type::POST("/", [](int) -> int { return 0; }).method(),
           http::verb::post);
  CHECK_EQ(route_type::PUT("/", [](int) -> int { return 0; }).method(),
           http::verb::put);
  CHECK_EQ(route_type::PATCH("/", [](int) -> int { return 0; }).method(),
           http::verb::patch);
  CHECK_EQ(route_type::DELETE_("/", [](int) -> int { return 0; }).method(),
           http::verb::delete_);
  CHECK_EQ(route_type::HEAD("/", [](int) -> int { return 0; }).method(),
           http::verb::head);
  CHECK_EQ(route_type::OPTIONS("/", [](int) -> int { return 0; }).method(),
           http::verb::options);
}

TEST_SUITE_END();
