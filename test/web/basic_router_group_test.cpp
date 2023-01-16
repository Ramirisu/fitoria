//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/basic_handler.hpp>
#include <fitoria/web/basic_middleware.hpp>
#include <fitoria/web/basic_router_group.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("basic_router_group");

TEST_CASE("basic")
{
  using router_group_type
      = basic_router_group<basic_middleware<int, int>, basic_handler<int, int>>;

  auto h = [](int) -> int { return 0; };
  auto l = [](int) -> int { return 1; };
  auto ag = [](int) -> int { return 2; };
  auto af = [](int) -> int { return 3; };

  auto rg
      = router_group_type("/ramirisu")
            .use(l)
            .route(http::verb::get, "/libraries", h)
            .route(http::verb::put, "/libraries", h)
            .sub_group(
                router_group_type("/gul")
                    .use(ag)
                    .route(http::verb::get, "/tags", h)
                    .route(router_group_type::router_type(http::verb::put,
                                                          "/tags", h))
                    .sub_group(router_group_type("/tags")
                                   .route(http::verb::get, "/{tag}", h)
                                   .route(http::verb::put, "/{tag}", h))
                    .sub_group(router_group_type("/branches")
                                   .route(http::verb::get, "/{branch}", h)
                                   .route(http::verb::put, "/{branch}", h)))
            .sub_group(
                router_group_type("/fitoria")
                    .use(af)
                    .route(http::verb::get, "/tags", h)
                    .route(router_group_type::router_type(http::verb::put,
                                                          "/tags", h))
                    .sub_group(router_group_type("/tags")
                                   .route(http::verb::get, "/{tag}", h)
                                   .route(http::verb::put, "/{tag}", h))
                    .sub_group(router_group_type("/branches")
                                   .route(http::verb::get, "/{branch}", h)
                                   .route(http::verb::put, "/{branch}", h)));

  const auto exp = std::vector<router_group_type::router_type> {
    { http::verb::get, "/ramirisu/libraries", { l }, { h } },
    { http::verb::put, "/ramirisu/libraries", { l }, { h } },
    { http::verb::get, "/ramirisu/gul/tags", { l, ag }, { h } },
    { http::verb::put, "/ramirisu/gul/tags", { l, ag }, { h } },
    { http::verb::get, "/ramirisu/gul/tags/{tag}", { l, ag }, { h } },
    { http::verb::put, "/ramirisu/gul/tags/{tag}", { l, ag }, { h } },
    { http::verb::get, "/ramirisu/gul/branches/{branch}", { l, ag }, { h } },
    { http::verb::put, "/ramirisu/gul/branches/{branch}", { l, ag }, { h } },
    { http::verb::get, "/ramirisu/fitoria/tags", { l, af }, { h } },
    { http::verb::put, "/ramirisu/fitoria/tags", { l, af }, { h } },
    { http::verb::get, "/ramirisu/fitoria/tags/{tag}", { l, af }, { h } },
    { http::verb::put, "/ramirisu/fitoria/tags/{tag}", { l, af }, { h } },
    { http::verb::get,
      "/ramirisu/fitoria/branches/{branch}",
      { l, af },
      { h } },
    { http::verb::put,
      "/ramirisu/fitoria/branches/{branch}",
      { l, af },
      { h } },
  };
  CHECK(range_equal(rg.routers(), exp, [](auto& lhs, auto& rhs) {
    if (lhs.method() == rhs.method() && lhs.path() == rhs.path()
        && lhs.handler()(0) == rhs.handler()(0)) {
      for (std::size_t i = 0; i < lhs.middlewares().size(); ++i) {
        if (lhs.middlewares()[i](0) != rhs.middlewares()[i](0)) {
          return false;
        }
      }
      return true;
    }
    return false;
  }));
}

TEST_SUITE_END();
