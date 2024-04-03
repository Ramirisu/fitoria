//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/web/route.hpp>

using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.route]");

namespace {

using req_type = int;
using res_type = int;
res_type handler(req_type)
{
  return 0;
}
}

TEST_CASE("method")
{
  CHECK_EQ(route::handle<"/">(http::verb::get, handler)
               .template build<req_type, res_type>()
               .method(),
           http::verb::get);
  CHECK_EQ(route::handle<"/">(http::verb::post, handler)
               .template build<req_type, res_type>()
               .method(),
           http::verb::post);
  CHECK_EQ(route::handle<"/">(http::verb::put, handler)
               .template build<req_type, res_type>()
               .method(),
           http::verb::put);
  CHECK_EQ(route::handle<"/">(http::verb::patch, handler)
               .template build<req_type, res_type>()
               .method(),
           http::verb::patch);
  CHECK_EQ(route::handle<"/">(http::verb::delete_, handler)
               .template build<req_type, res_type>()
               .method(),
           http::verb::delete_);
  CHECK_EQ(route::handle<"/">(http::verb::options, handler)
               .template build<req_type, res_type>()
               .method(),
           http::verb::options);

  CHECK_EQ(
      route::get<"/">(handler).template build<req_type, res_type>().method(),
      http::verb::get);
  CHECK_EQ(
      route::post<"/">(handler).template build<req_type, res_type>().method(),
      http::verb::post);
  CHECK_EQ(
      route::put<"/">(handler).template build<req_type, res_type>().method(),
      http::verb::put);
  CHECK_EQ(
      route::patch<"/">(handler).template build<req_type, res_type>().method(),
      http::verb::patch);
  CHECK_EQ(route::delete_<"/">(handler)
               .template build<req_type, res_type>()
               .method(),
           http::verb::delete_);
  CHECK_EQ(
      route::head<"/">(handler).template build<req_type, res_type>().method(),
      http::verb::head);
  CHECK_EQ(route::options<"/">(handler)
               .template build<req_type, res_type>()
               .method(),
           http::verb::options);

  CHECK_EQ(
      route::any<"/">(handler).template build<req_type, res_type>().method(),
      http::verb::unknown);
}

TEST_CASE("rebind_parent")
{
  const auto r1 = route::handle<"/v1">(http::verb::get, handler);
  CHECK_EQ(r1.template build<req_type, res_type>().method(), http::verb::get);
  CHECK_EQ(r1.template build<req_type, res_type>().matcher().pattern(), "/v1");

  auto r2 = r1.rebind_parent<"/api">(std::make_shared<state_map>(), {});
  CHECK_EQ(r2.template build<req_type, res_type>().method(), http::verb::get);
  CHECK_EQ(r2.template build<req_type, res_type>().matcher().pattern(),
           "/api/v1");
}

TEST_SUITE_END();
