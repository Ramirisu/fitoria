//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/http_server/router_tree.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("router_tree");

TEST_CASE("try_insert")
{
  struct test_handler_trait {
    using handler_t = std::function<int()>;
    using handlers_t = std::vector<handler_t>;
    struct handler_compare_t {
      bool operator()(const handler_t& lhs, const handler_t& rhs) const
      {
        return lhs() == rhs();
      }
    };
  };
  using router_tree_type = basic_router_tree<test_handler_trait>;
  using exp_t = expected<void, router_error>;

  auto r = [=](verb method, std::string path) {
    return router_tree_type::router_type(method, std::move(path),
                                         handlers_t<test_handler_trait> {});
  };

  router_tree_type rt;
  CHECK_EQ(rt.try_insert(r(verb::get, "")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(verb::get, "/")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(verb::get, "/{")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(verb::get, "/}")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(verb::get, "//")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(verb::get, "/ramirisu/")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(verb::get, "/ramirisu/{")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(verb::get, "/ramirisu/}")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(verb::get, "/ramirisu//")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(verb::get, "/ramirisu")), exp_t());
  CHECK_EQ(rt.try_insert(r(verb::get, "/ramirisu")),
           exp_t(unexpect, router_error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(verb::put, "/ramirisu")), exp_t());
  CHECK_EQ(rt.try_insert(r(verb::put, "/ramirisu")),
           exp_t(unexpect, router_error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(verb::get, "/ramirisu/{repo}")), exp_t());
  CHECK_EQ(rt.try_insert(r(verb::get, "/ramirisu/{r}")),
           exp_t(unexpect, router_error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(verb::get, "/ramirisu/{repo}/{branch}")), exp_t());
  CHECK_EQ(rt.try_insert(r(verb::get, "/ramirisu/{r}/{b}")),
           exp_t(unexpect, router_error::route_already_exists));
}

TEST_CASE("try_find")
{
  struct test_handler_trait {
    using handler_t = std::function<int()>;
    using handlers_t = std::vector<handler_t>;
    struct handler_compare_t;
  };
  using router_tree_type = basic_router_tree<test_handler_trait>;

  auto r = [=](verb method, std::string path, int exp) {
    return router_tree_type::router_type(
        method, std::move(path),
        handlers_t<test_handler_trait> { [=]() { return exp; } });
  };

  router_tree_type rt;
  rt.try_insert(r(verb::get, "/r", 0));
  rt.try_insert(r(verb::put, "/r", 1));
  rt.try_insert(r(verb::get, "/r/x", 10));
  rt.try_insert(r(verb::put, "/r/x", 11));
  rt.try_insert(r(verb::get, "/r/{x}", 12));
  rt.try_insert(r(verb::put, "/r/{x}", 13));
  rt.try_insert(r(verb::get, "/r/x/y", 20));
  rt.try_insert(r(verb::put, "/r/x/y", 21));
  rt.try_insert(r(verb::get, "/r/{x}/{y}", 22));
  rt.try_insert(r(verb::put, "/r/{x}/{y}", 23));

  CHECK_EQ(rt.try_find(verb::get, "/r")->handlers()[0](), 0);
  CHECK_EQ(rt.try_find(verb::put, "/r")->handlers()[0](), 1);
  CHECK_EQ(rt.try_find(verb::get, "/r/x")->handlers()[0](), 10);
  CHECK_EQ(rt.try_find(verb::put, "/r/x")->handlers()[0](), 11);
  CHECK_EQ(rt.try_find(verb::get, "/r/xx")->handlers()[0](), 12);
  CHECK_EQ(rt.try_find(verb::put, "/r/xx")->handlers()[0](), 13);
  CHECK_EQ(rt.try_find(verb::get, "/r/x/y")->handlers()[0](), 20);
  CHECK_EQ(rt.try_find(verb::put, "/r/x/y")->handlers()[0](), 21);
  CHECK_EQ(rt.try_find(verb::get, "/r/xx/y")->handlers()[0](), 22);
  CHECK_EQ(rt.try_find(verb::put, "/r/xx/y")->handlers()[0](), 23);
  CHECK_EQ(rt.try_find(verb::get, "/r/x/yy")->handlers()[0](), 22);
  CHECK_EQ(rt.try_find(verb::put, "/r/x/yy")->handlers()[0](), 23);
}

TEST_SUITE_END();
