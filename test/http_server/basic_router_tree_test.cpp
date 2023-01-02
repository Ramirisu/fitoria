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

namespace {

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

}

TEST_CASE("try_insert")
{
  using router_tree_type = basic_router_tree<test_handler_trait>;
  using exp_t = expected<void, error>;

  auto r = [=](http::verb method, std::string path) {
    return router_tree_type::router_type(method, std::move(path),
                                         handler_t<test_handler_trait> {});
  };

  router_tree_type rt;
  CHECK_EQ(rt.try_insert(r(http::verb::get, "")),
           exp_t(unexpect, error::route_parse_error));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/")),
           exp_t(unexpect, error::route_parse_error));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/{")),
           exp_t(unexpect, error::route_parse_error));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/}")),
           exp_t(unexpect, error::route_parse_error));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "//")),
           exp_t(unexpect, error::route_parse_error));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/ramirisu/")),
           exp_t(unexpect, error::route_parse_error));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/ramirisu/{")),
           exp_t(unexpect, error::route_parse_error));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/ramirisu/}")),
           exp_t(unexpect, error::route_parse_error));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/ramirisu//")),
           exp_t(unexpect, error::route_parse_error));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/ramirisu")), exp_t());
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/ramirisu")),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(http::verb::put, "/ramirisu")), exp_t());
  CHECK_EQ(rt.try_insert(r(http::verb::put, "/ramirisu")),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/ramirisu/{repo}")), exp_t());
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/ramirisu/{r}")),
           exp_t(unexpect, error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/ramirisu/{repo}/{branch}")),
           exp_t());
  CHECK_EQ(rt.try_insert(r(http::verb::get, "/ramirisu/{r}/{b}")),
           exp_t(unexpect, error::route_already_exists));
}

TEST_CASE("try_find")
{
  using router_tree_type = basic_router_tree<test_handler_trait>;

  auto r = [=](http::verb method, std::string path, int exp) {
    return router_tree_type::router_type(
        method, std::move(path),
        handler_t<test_handler_trait> { [=]() { return exp; } });
  };

  router_tree_type rt;
  rt.try_insert(r(http::verb::get, "/r", 0));
  rt.try_insert(r(http::verb::put, "/r", 1));
  rt.try_insert(r(http::verb::get, "/r/x", 10));
  rt.try_insert(r(http::verb::put, "/r/x", 11));
  rt.try_insert(r(http::verb::get, "/r/{x}", 12));
  rt.try_insert(r(http::verb::put, "/r/{x}", 13));
  rt.try_insert(r(http::verb::get, "/r/x/y", 20));
  rt.try_insert(r(http::verb::put, "/r/x/y", 21));
  rt.try_insert(r(http::verb::get, "/r/{x}/{y}", 22));
  rt.try_insert(r(http::verb::put, "/r/{x}/{y}", 23));

  CHECK_EQ(rt.try_find(http::verb::get, "/r")->handler()(), 0);
  CHECK_EQ(rt.try_find(http::verb::put, "/r")->handler()(), 1);
  CHECK_EQ(rt.try_find(http::verb::get, "/r/x")->handler()(), 10);
  CHECK_EQ(rt.try_find(http::verb::put, "/r/x")->handler()(), 11);
  CHECK_EQ(rt.try_find(http::verb::get, "/r/xx")->handler()(), 12);
  CHECK_EQ(rt.try_find(http::verb::put, "/r/xx")->handler()(), 13);
  CHECK_EQ(rt.try_find(http::verb::get, "/r/x/y")->handler()(), 20);
  CHECK_EQ(rt.try_find(http::verb::put, "/r/x/y")->handler()(), 21);
  CHECK_EQ(rt.try_find(http::verb::get, "/r/xx/y")->handler()(), 22);
  CHECK_EQ(rt.try_find(http::verb::put, "/r/xx/y")->handler()(), 23);
  CHECK_EQ(rt.try_find(http::verb::get, "/r/x/yy")->handler()(), 22);
  CHECK_EQ(rt.try_find(http::verb::put, "/r/x/yy")->handler()(), 23);
}

TEST_SUITE_END();
