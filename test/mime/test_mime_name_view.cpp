//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/mime/name_view.hpp>

using namespace fitoria;
using namespace fitoria::mime;

TEST_SUITE_BEGIN("[fitoria.mime.name_view]");

TEST_CASE("to_string")
{
  CHECK_EQ(mime::name_view("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ").to_string(),
           "0123456789abcdefghijklmnopqrstuvwxyz");
}

TEST_CASE("compare")
{
  CHECK_EQ(mime::name_view("abc"), mime::name_view("ABC"));
}

TEST_CASE("format")
{
  CHECK_EQ(fmt::format("{}", mime::name_view("application")), "application");
}

TEST_SUITE_END();
