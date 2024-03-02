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

namespace {
int handler(int)
{
  return 0;
}
}

TEST_CASE("method")
{
  CHECK_EQ(route::handle<"/">(http::verb::get, handler).build().method(),
           http::verb::get);
  CHECK_EQ(route::handle<"/">(http::verb::post, handler).build().method(),
           http::verb::post);
  CHECK_EQ(route::handle<"/">(http::verb::put, handler).build().method(),
           http::verb::put);
  CHECK_EQ(route::handle<"/">(http::verb::patch, handler).build().method(),
           http::verb::patch);
  CHECK_EQ(route::handle<"/">(http::verb::delete_, handler).build().method(),
           http::verb::delete_);
  CHECK_EQ(route::handle<"/">(http::verb::options, handler).build().method(),
           http::verb::options);

  CHECK_EQ(route::GET<"/">(handler).build().method(), http::verb::get);
  CHECK_EQ(route::POST<"/">(handler).build().method(), http::verb::post);
  CHECK_EQ(route::PUT<"/">(handler).build().method(), http::verb::put);
  CHECK_EQ(route::PATCH<"/">(handler).build().method(), http::verb::patch);
  CHECK_EQ(route::DELETE_<"/">(handler).build().method(), http::verb::delete_);
  CHECK_EQ(route::HEAD<"/">(handler).build().method(), http::verb::head);
  CHECK_EQ(route::OPTIONS<"/">(handler).build().method(), http::verb::options);

  CHECK_EQ(route::any<"/">(handler).build().method(), http::verb::unknown);
}

TEST_CASE("rebind_parent")
{
  const auto r1 = route::handle<"/v1">(http::verb::get, handler);
  CHECK_EQ(r1.build().method(), http::verb::get);
  CHECK_EQ(r1.build().matcher().pattern(), "/v1");

  auto r2 = r1.rebind_parent<"/api">({}, {});
  CHECK_EQ(r2.build().method(), http::verb::get);
  CHECK_EQ(r2.build().matcher().pattern(), "/api/v1");
}

TEST_SUITE_END();
