//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/as_form.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("web.as_form");

TEST_CASE("as_form")
{
  {
    CHECK_EQ(as_form("%%").error(),
             make_error_code(boost::urls::error::missing_pct_hexdig));
  }
  {
    auto form = as_form(R"(name=Rina%20Hidaka&birth=1994%2F06%2F15)");
    CHECK_EQ(form->get("name"), "Rina Hidaka");
    CHECK_EQ(form->get("birth"), "1994/06/15");
  }
}

TEST_SUITE_END();
