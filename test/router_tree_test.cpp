//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the ramirisu Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.ramirisu.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/router/router_tree.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("router_tree");

TEST_CASE("try_insert")
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

  auto r = [=](methods method, std::string path) {
    return rt_type::router_type(method, std::move(path), {});
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

TEST_CASE("try_find")
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

  auto r = [=](methods method, std::string path, int exp) {
    return rt_type::router_type(method, std::move(path),
                                { [=]() { return exp; } });
  };

  rt_type rt;
  rt.try_insert(r(methods::get, "/r", 0));
  rt.try_insert(r(methods::put, "/r", 1));
  rt.try_insert(r(methods::get, "/r/x", 10));
  rt.try_insert(r(methods::put, "/r/x", 11));
  rt.try_insert(r(methods::get, "/r/:x", 12));
  rt.try_insert(r(methods::put, "/r/:x", 13));
  rt.try_insert(r(methods::get, "/r/x/y", 20));
  rt.try_insert(r(methods::put, "/r/x/y", 21));
  rt.try_insert(r(methods::get, "/r/:x/:y", 22));
  rt.try_insert(r(methods::put, "/r/:x/:y", 23));

  CHECK_EQ(rt.try_find(methods::get, "/r")->handlers()[0](), 0);
  CHECK_EQ(rt.try_find(methods::put, "/r")->handlers()[0](), 1);
  CHECK_EQ(rt.try_find(methods::get, "/r/x")->handlers()[0](), 10);
  CHECK_EQ(rt.try_find(methods::put, "/r/x")->handlers()[0](), 11);
  CHECK_EQ(rt.try_find(methods::get, "/r/xx")->handlers()[0](), 12);
  CHECK_EQ(rt.try_find(methods::put, "/r/xx")->handlers()[0](), 13);
  CHECK_EQ(rt.try_find(methods::get, "/r/x/y")->handlers()[0](), 20);
  CHECK_EQ(rt.try_find(methods::put, "/r/x/y")->handlers()[0](), 21);
  CHECK_EQ(rt.try_find(methods::get, "/r/xx/y")->handlers()[0](), 22);
  CHECK_EQ(rt.try_find(methods::put, "/r/xx/y")->handlers()[0](), 23);
  CHECK_EQ(rt.try_find(methods::get, "/r/x/yy")->handlers()[0](), 22);
  CHECK_EQ(rt.try_find(methods::put, "/r/x/yy")->handlers()[0](), 23);
}

TEST_SUITE_END();
