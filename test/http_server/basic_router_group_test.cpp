//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/http_server/router_group.hpp>

#include <functional>

using namespace fitoria;

TEST_SUITE_BEGIN("router_group");

TEST_CASE("basic")
{
  struct test_handler_trait {
    using middleware_t = std::function<int()>;
    using middlewares_t = std::vector<middleware_t>;
    struct middleware_compare_t {
      bool operator()(const middleware_t& lhs, const middleware_t& rhs) const
      {
        return lhs() == rhs();
      }
    };
    using handler_t = std::function<int()>;
    struct handler_compare_t {
      bool operator()(const handler_t& lhs, const handler_t& rhs) const
      {
        return lhs() == rhs();
      }
    };
  };

  using router_group_type = basic_router_group<test_handler_trait>;

  auto h = []() -> int { return 0; };
  auto l = []() -> int { return 1; };
  auto ag = []() -> int { return 2; };
  auto af = []() -> int { return 3; };

  auto rg
      = router_group_type("/ramirisu")
            .use(l)
            .route(http::verb::get, "/libraries", h)
            .route(http::verb::put, "/libraries", h)
            .sub_group(
                router_group_type("/gul")
                    .use(ag)
                    .route(http::verb::get, "/tags", h)
                    .route(http::verb::put, "/tags", h)
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
                    .route(http::verb::put, "/tags", h)
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
  CHECK(range_equal(rg.get_all_routers(), exp));
}

TEST_SUITE_END();
