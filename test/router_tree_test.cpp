//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the ramirisu Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.ramirisu.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/router_tree.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("router_tree");

TEST_CASE("build tree")
{
  struct handler_trait {
    using handler_type = std::function<int()>;
    struct compare {
      bool operator()(const handler_type& lhs, const handler_type& rhs) const
      {
        return lhs() == rhs();
      }
    };
  };
  using rt_type = router_tree<handler_trait>;

  using exp_t = expected<void, router_error>;
  auto m = []() { return 0; };
  auto h = []() { return 1; };

  auto r = [=](methods method, std::string path) {
    return rt_type::router_type(method, std::move(path), { m }, h);
  };

  rt_type rt;
  CHECK_EQ(rt.try_insert(r(methods::get, "")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(methods::get, "/")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(methods::get, "/:")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(methods::get, "//")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(methods::get, "/ramirisu/")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(methods::get, "/ramirisu/:")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(methods::get, "/ramirisu//")),
           exp_t(unexpect, router_error::parse_path_error));
  CHECK_EQ(rt.try_insert(r(methods::get, "/ramirisu")), exp_t());
  CHECK_EQ(rt.try_insert(r(methods::get, "/ramirisu")),
           exp_t(unexpect, router_error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(methods::put, "/ramirisu")), exp_t());
  CHECK_EQ(rt.try_insert(r(methods::put, "/ramirisu")),
           exp_t(unexpect, router_error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(methods::get, "/ramirisu/:repo")), exp_t());
  CHECK_EQ(rt.try_insert(r(methods::get, "/ramirisu/:r")),
           exp_t(unexpect, router_error::route_already_exists));
  CHECK_EQ(rt.try_insert(r(methods::get, "/ramirisu/:repo/:branch")), exp_t());
  CHECK_EQ(rt.try_insert(r(methods::get, "/ramirisu/:r/:b")),
           exp_t(unexpect, router_error::route_already_exists));
}

TEST_SUITE_END();
