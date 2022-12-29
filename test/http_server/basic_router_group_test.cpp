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
            .route(verb::get, "/libraries", h)
            .route(verb::put, "/libraries", h)
            .sub_group(router_group_type("/gul")
                           .use(ag)
                           .route(verb::get, "/tags", h)
                           .route(verb::put, "/tags", h)
                           .sub_group(router_group_type("/tags")
                                          .route(verb::get, "/{tag}", h)
                                          .route(verb::put, "/{tag}", h))
                           .sub_group(router_group_type("/branches")
                                          .route(verb::get, "/{branch}", h)
                                          .route(verb::put, "/{branch}", h)))
            .sub_group(router_group_type("/fitoria")
                           .use(af)
                           .route(verb::get, "/tags", h)
                           .route(verb::put, "/tags", h)
                           .sub_group(router_group_type("/tags")
                                          .route(verb::get, "/{tag}", h)
                                          .route(verb::put, "/{tag}", h))
                           .sub_group(router_group_type("/branches")
                                          .route(verb::get, "/{branch}", h)
                                          .route(verb::put, "/{branch}", h)));

  const auto exp = std::vector<router_group_type::router_type> {
    { verb::get, "/ramirisu/libraries", { l }, { h } },
    { verb::put, "/ramirisu/libraries", { l }, { h } },
    { verb::get, "/ramirisu/gul/tags", { l, ag }, { h } },
    { verb::put, "/ramirisu/gul/tags", { l, ag }, { h } },
    { verb::get, "/ramirisu/gul/tags/{tag}", { l, ag }, { h } },
    { verb::put, "/ramirisu/gul/tags/{tag}", { l, ag }, { h } },
    { verb::get, "/ramirisu/gul/branches/{branch}", { l, ag }, { h } },
    { verb::put, "/ramirisu/gul/branches/{branch}", { l, ag }, { h } },
    { verb::get, "/ramirisu/fitoria/tags", { l, af }, { h } },
    { verb::put, "/ramirisu/fitoria/tags", { l, af }, { h } },
    { verb::get, "/ramirisu/fitoria/tags/{tag}", { l, af }, { h } },
    { verb::put, "/ramirisu/fitoria/tags/{tag}", { l, af }, { h } },
    { verb::get, "/ramirisu/fitoria/branches/{branch}", { l, af }, { h } },
    { verb::put, "/ramirisu/fitoria/branches/{branch}", { l, af }, { h } },
  };
  CHECK(range_equal(rg.get_all_routers(), exp));
}

TEST_SUITE_END();
