//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/web/path_matcher.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.path_matcher]");

TEST_CASE("tokens")
{
  CHECK_EQ(path_matcher("").tokens(), path_tokens_t {});
  CHECK_EQ(
      path_matcher("/w/x/y/z").tokens(),
      path_tokens_t { path_token_t { path_token_kind::static_, "/w/x/y/z" } });
  CHECK_EQ(path_matcher("/{p1}").tokens(),
           path_tokens_t { path_token_t { path_token_kind::static_, "/" },
                           path_token_t { path_token_kind::param, "p1" } });
  CHECK_EQ(path_matcher("/{p1}/x/{p3}/z").tokens(),
           path_tokens_t { path_token_t { path_token_kind::static_, "/" },
                           path_token_t { path_token_kind::param, "p1" },
                           path_token_t { path_token_kind::static_, "/x/" },
                           path_token_t { path_token_kind::param, "p3" },
                           path_token_t { path_token_kind::static_, "/z" } });
  CHECK_EQ(
      path_matcher("/w/x/y/z/#abc").tokens(),
      path_tokens_t { path_token_t { path_token_kind::static_, "/w/x/y/z/" },
                      path_token_t { path_token_kind::wildcard, "abc" } });
  CHECK_EQ(path_matcher("/{p1}/#abc").tokens(),
           path_tokens_t { path_token_t { path_token_kind::static_, "/" },
                           path_token_t { path_token_kind::param, "p1" },
                           path_token_t { path_token_kind::static_, "/" },
                           path_token_t { path_token_kind::wildcard, "abc" } });
}

TEST_CASE("match")
{
  using match_type = std::vector<std::pair<std::string, std::string>>;

  CHECK_EQ(path_matcher("").match(""), match_type {});
  CHECK_EQ(path_matcher("/w/x/y/z").match("/w/x/y/z"), match_type {});
  CHECK_EQ(path_matcher("/{p1}/x/{p3}/z").match("/w/x/y/z"),
           match_type {
               { "p1", "w" },
               { "p3", "y" },
           });
  CHECK_EQ(path_matcher("/{p1}/x/{p3}/z/#wildcard").match("/w/x/y/z/abc"),
           match_type {
               { "p1", "w" },
               { "p3", "y" },
               { "wildcard", "abc" },
           });

  CHECK(!path_matcher("/w").match(""));
  CHECK(!path_matcher("/w").match("/w/x"));
  CHECK(!path_matcher("/{p1}").match("/w/x"));
}

TEST_SUITE_END();
