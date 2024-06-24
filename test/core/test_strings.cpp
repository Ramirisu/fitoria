//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/core/strings.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("[fitoria.core.strings]");

TEST_CASE("iequals")
{
  CHECK(iequals("0123456789abcdefghijklmnopqrstuvwxyz",
                "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
  CHECK(!iequals("0123456789a", "0123456789"));
  CHECK(!iequals("0123456789a", "0123456789B"));
}

TEST_CASE("ltrim")
{
  CHECK_EQ(ltrim("     abc     "), "abc     ");
}

TEST_CASE("rtrim")
{
  CHECK_EQ(rtrim("     abc     "), "     abc");
}

TEST_CASE("trim")
{
  CHECK_EQ(trim("     abc     "), "abc");
}

TEST_CASE("split_of")
{
  CHECK_EQ(split_of("key0=value0; key1=value1; key2=value2", ";"),
           std::vector<std::string_view> {
               "key0=value0", "key1=value1", "key2=value2" });
  CHECK_EQ(split_of("key0=value0; key1=value1& key2=value2", ";&"),
           std::vector<std::string_view> {
               "key0=value0", "key1=value1", "key2=value2" });
}

TEST_SUITE_END();
