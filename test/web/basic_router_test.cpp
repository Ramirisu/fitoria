//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/any_routable.hpp>
#include <fitoria/web/basic_router.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("web.basic_router");

namespace {

using router_type = basic_router<any_routable<int, int>>;

}

TEST_CASE("try_insert")
{
  using exp_t = expected<void, error>;

  auto r = [=](http::verb method, std::string path) {
    return router_type::route_type(
        method, match_pattern::from_pattern(std::move(path)).value(), {},
        [](int) { return 0; });
  };

  router_type rt;
  CHECK_EQ(rt.try_insert(r(http::verb::get, "")), exp_t());
  CHECK_EQ(rt.try_insert(r(http::verb::get, "")),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/")), exp_t());
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/")),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "//")), exp_t());
  CHECK_EQ(rt.try_insert(r(http::verb::get, "//")),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/api")), exp_t());
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/api")),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(http::verb::put, "/api")), exp_t());
  CHECK_EQ(rt.try_insert(r(http::verb::put, "/api")),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/api/")), exp_t());
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/api/")),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/api//")), exp_t());
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/api//")),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/api/{x0}")), exp_t());
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/api/{x1}")),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/api/{x0}/{y0}")), exp_t());
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/api/{x1}/{y1}")),
           exp_t(unexpect, error::route_already_exists));
}

TEST_CASE("try_find")
{
  auto r = [=](http::verb method, std::string path, int exp) {
    return router_type::route_type(
        method, match_pattern::from_pattern(std::move(path)).value(), {},
        [=](int) -> int { return exp; });
  };

  router_type rt;
  rt.try_insert(r(http::verb::get, "/api/v1/x", 10));
  rt.try_insert(r(http::verb::put, "/api/v1/x", 11));
  rt.try_insert(r(http::verb::get, "/api/v1/{x}", 12));
  rt.try_insert(r(http::verb::put, "/api/v1/{x}", 13));
  rt.try_insert(r(http::verb::get, "/api/v1/x/y", 20));
  rt.try_insert(r(http::verb::put, "/api/v1/x/y", 21));
  rt.try_insert(r(http::verb::get, "/api/v1/{x}/{y}", 22));
  rt.try_insert(r(http::verb::put, "/api/v1/{x}/{y}", 23));

  CHECK_EQ(rt.try_find(http::verb::get, ""),
           fitoria::unexpected { make_error_code(error::route_not_exists) });
  CHECK_EQ(rt.try_find(http::verb::get, "a"),
           fitoria::unexpected { make_error_code(error::route_not_exists) });
  CHECK_EQ(rt.try_find(http::verb::get, "/"),
           fitoria::unexpected { make_error_code(error::route_not_exists) });
  CHECK_EQ(rt.try_find(http::verb::get, "/a"),
           fitoria::unexpected { make_error_code(error::route_not_exists) });
  CHECK_EQ(rt.try_find(http::verb::get, "/api"),
           fitoria::unexpected { make_error_code(error::route_not_exists) });
  CHECK_EQ(rt.try_find(http::verb::get, "/api/"),
           fitoria::unexpected { make_error_code(error::route_not_exists) });
  CHECK_EQ(rt.try_find(http::verb::get, "/api/v1"),
           fitoria::unexpected { make_error_code(error::route_not_exists) });
  CHECK_EQ(rt.try_find(http::verb::get, "/api/v1/xxx/yyy/z"),
           fitoria::unexpected { make_error_code(error::route_not_exists) });
  CHECK_EQ(rt.try_find(http::verb::patch, "/api/v1/x"),
           fitoria::unexpected { make_error_code(error::route_not_exists) });
  CHECK_EQ(rt.try_find(http::verb::get, "/api/v1/x")->operator()(0), 10);
  CHECK_EQ(rt.try_find(http::verb::put, "/api/v1/x")->operator()(0), 11);
  CHECK_EQ(rt.try_find(http::verb::get, "/api/v1/xx")->operator()(0), 12);
  CHECK_EQ(rt.try_find(http::verb::put, "/api/v1/xx")->operator()(0), 13);
  CHECK_EQ(rt.try_find(http::verb::get, "/api/v1/x/y")->operator()(0), 20);
  CHECK_EQ(rt.try_find(http::verb::put, "/api/v1/x/y")->operator()(0), 21);
  CHECK_EQ(rt.try_find(http::verb::get, "/api/v1/xx/y")->operator()(0), 22);
  CHECK_EQ(rt.try_find(http::verb::put, "/api/v1/xx/y")->operator()(0), 23);
  CHECK_EQ(rt.try_find(http::verb::get, "/api/v1/x/yy")->operator()(0), 22);
  CHECK_EQ(rt.try_find(http::verb::put, "/api/v1/x/yy")->operator()(0), 23);
}

TEST_SUITE_END();
