//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/path_matcher.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("web.path_matcher");

TEST_CASE("tokens")
{
  using token_kind = path_matcher::token_kind;
  using tokens_t = path_matcher::tokens_t;
  using token_t = path_matcher::token_t;

  CHECK_EQ(path_matcher("").tokens(), tokens_t {});
  CHECK_EQ(path_matcher("/w/x/y/z").tokens(),
           tokens_t { token_t { token_kind::static_, "/w/x/y/z" } });
  CHECK_EQ(path_matcher("/{p1}").tokens(),
           tokens_t { token_t { token_kind::static_, "/" },
                      token_t { token_kind::named_param, "p1" } });
  CHECK_EQ(path_matcher("/{p1}/x/{p3}/z").tokens(),
           tokens_t { token_t { token_kind::static_, "/" },
                      token_t { token_kind::named_param, "p1" },
                      token_t { token_kind::static_, "/x/" },
                      token_t { token_kind::named_param, "p3" },
                      token_t { token_kind::static_, "/z" } });
  CHECK_EQ(path_matcher("/w/x/y/z/#abc").tokens(),
           tokens_t { token_t { token_kind::static_, "/w/x/y/z/" },
                      token_t { token_kind::wildcard, "abc" } });
  CHECK_EQ(path_matcher("/{p1}/#abc").tokens(),
           tokens_t { token_t { token_kind::static_, "/" },
                      token_t { token_kind::named_param, "p1" },
                      token_t { token_kind::static_, "/" },
                      token_t { token_kind::wildcard, "abc" } });
}

TEST_CASE("match")
{
  CHECK_EQ(path_matcher("").match(""), query_map());
  CHECK_EQ(path_matcher("/w/x/y/z").match("/w/x/y/z"), query_map());
  CHECK_EQ(path_matcher("/{p1}/x/{p3}/z").match("/w/x/y/z"),
           query_map {
               { "p1", "w" },
               { "p3", "y" },
           });
  CHECK_EQ(path_matcher("/{p1}/x/{p3}/z/#wildcard").match("/w/x/y/z/abc"),
           query_map {
               { "p1", "w" },
               { "p3", "y" },
               { "wildcard", "abc" },
           });

  CHECK(!path_matcher("/w").match(""));
  CHECK(!path_matcher("/w").match("/w/x"));
  CHECK(!path_matcher("/{p1}").match("/w/x"));
}

TEST_SUITE_END();
