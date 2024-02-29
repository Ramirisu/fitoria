//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/routable.hpp>
#include <fitoria/web/router.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("web.router");

namespace {

using router_type = router<int, int>;

template <basic_fixed_string Path>
auto r(http::verb method, int value = 0)
{
  return router_type::route_type(
      routable(method, pattern_matcher(Path), {}, [=](int) { return value; }));
}
}

TEST_CASE("try_insert")
{
  using exp_t = expected<void, error>;

  router_type rt;
  CHECK_EQ(rt.try_insert(r<"">(http::verb::get)), exp_t());
  CHECK_EQ(rt.try_insert(r<"">(http::verb::get)),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r<"/">(http::verb::get)), exp_t());
  CHECK_EQ(rt.try_insert(r<"/">(http::verb::get)),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r<"//">(http::verb::get)), exp_t());
  CHECK_EQ(rt.try_insert(r<"//">(http::verb::get)),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r<"/api">(http::verb::get)), exp_t());
  CHECK_EQ(rt.try_insert(r<"/api">(http::verb::get)),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r<"/api">(http::verb::put)), exp_t());
  CHECK_EQ(rt.try_insert(r<"/api">(http::verb::put)),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r<"/api/">(http::verb::get)), exp_t());
  CHECK_EQ(rt.try_insert(r<"/api/">(http::verb::get)),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r<"/api//">(http::verb::get)), exp_t());
  CHECK_EQ(rt.try_insert(r<"/api//">(http::verb::get)),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r<"/api/{x0}">(http::verb::get)), exp_t());
  CHECK_EQ(rt.try_insert(r<"/api/{x1}">(http::verb::get)),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r<"/api/{x0}/{y0}">(http::verb::get)), exp_t());
  CHECK_EQ(rt.try_insert(r<"/api/{x1}/{y1}">(http::verb::get)),
           exp_t(unexpect, error::route_already_exists));
}

TEST_CASE("try_find")
{
  router_type rt;
  rt.try_insert(r<"/api/v1/x">(http::verb::get, 10));
  rt.try_insert(r<"/api/v1/x">(http::verb::put, 11));
  rt.try_insert(r<"/api/v1/{x}">(http::verb::get, 12));
  rt.try_insert(r<"/api/v1/{x}">(http::verb::put, 13));
  rt.try_insert(r<"/api/v1/x/y">(http::verb::get, 20));
  rt.try_insert(r<"/api/v1/x/y">(http::verb::put, 21));
  rt.try_insert(r<"/api/v1/{x}/{y}">(http::verb::get, 22));
  rt.try_insert(r<"/api/v1/{x}/{y}">(http::verb::put, 23));

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
