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
      = route::handle<"/v1">(http::verb::get, [](int) -> int { return 0; });
  CHECK_EQ(r1.build().method(), http::verb::get);
  CHECK_EQ(r1.build().matcher().pattern(), "/v1");

  auto r2 = r1.rebind_parent<"/api">({}, {});
  CHECK_EQ(r2.build().method(), http::verb::get);
  CHECK_EQ(r2.build().matcher().pattern(), "/api/v1");
}

TEST_CASE("methods")
{
  CHECK_EQ(route::GET<"/">([](int) -> int { return 0; }).build().method(),
           http::verb::get);
  CHECK_EQ(route::POST<"/">([](int) -> int { return 0; }).build().method(),
           http::verb::post);
  CHECK_EQ(route::PUT<"/">([](int) -> int { return 0; }).build().method(),
           http::verb::put);
  CHECK_EQ(route::PATCH<"/">([](int) -> int { return 0; }).build().method(),
           http::verb::patch);
  CHECK_EQ(route::DELETE_<"/">([](int) -> int { return 0; }).build().method(),
           http::verb::delete_);
  CHECK_EQ(route::HEAD<"/">([](int) -> int { return 0; }).build().method(),
           http::verb::head);
  CHECK_EQ(route::OPTIONS<"/">([](int) -> int { return 0; }).build().method(),
           http::verb::options);
}

TEST_SUITE_END();
