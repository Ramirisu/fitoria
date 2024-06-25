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

TEST_CASE("cmp_eq_ci")
{
  CHECK(cmp_eq_ci("0123456789abcdefghijklmnopqrstuvwxyz",
                  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
  CHECK(!cmp_eq_ci("0123456789a", "0123456789"));
  CHECK(!cmp_eq_ci("0123456789a", "0123456789B"));
}

TEST_CASE("cmp_tw_ci")
{
  CHECK_EQ(cmp_tw_ci("", ""), std::strong_ordering::equal);
  CHECK_EQ(cmp_tw_ci("", "abc"), std::strong_ordering::less);
  CHECK_EQ(cmp_tw_ci("", "ABC"), std::strong_ordering::less);
  CHECK_EQ(cmp_tw_ci("abc", ""), std::strong_ordering::greater);
  CHECK_EQ(cmp_tw_ci("ABC", ""), std::strong_ordering::greater);
  CHECK_EQ(cmp_tw_ci("abc", "abc"), std::strong_ordering::equal);
  CHECK_EQ(cmp_tw_ci("abc", "ABC"), std::strong_ordering::equal);
  CHECK_EQ(cmp_tw_ci("ABC", "abc"), std::strong_ordering::equal);
  CHECK_EQ(cmp_tw_ci("ab", "abc"), std::strong_ordering::less);
  CHECK_EQ(cmp_tw_ci("ab", "ABC"), std::strong_ordering::less);
  CHECK_EQ(cmp_tw_ci("AB", "abc"), std::strong_ordering::less);
  CHECK_EQ(cmp_tw_ci("abc", "ab"), std::strong_ordering::greater);
  CHECK_EQ(cmp_tw_ci("abc", "AB"), std::strong_ordering::greater);
  CHECK_EQ(cmp_tw_ci("ABC", "ab"), std::strong_ordering::greater);
  CHECK_EQ(cmp_tw_ci("abcyz", "abxyz"), std::strong_ordering::less);
  CHECK_EQ(cmp_tw_ci("abcyz", "ABXYZ"), std::strong_ordering::less);
  CHECK_EQ(cmp_tw_ci("ABCYZ", "abxyz"), std::strong_ordering::less);
  CHECK_EQ(cmp_tw_ci("abxyz", "abcyz"), std::strong_ordering::greater);
  CHECK_EQ(cmp_tw_ci("ABXYZ", "abcyz"), std::strong_ordering::greater);
  CHECK_EQ(cmp_tw_ci("abxyz", "ABCYZ"), std::strong_ordering::greater);
  CHECK_EQ(cmp_tw_ci("abc", "abxyz"), std::strong_ordering::less);
  CHECK_EQ(cmp_tw_ci("abc", "ABXYZ"), std::strong_ordering::less);
  CHECK_EQ(cmp_tw_ci("ABC", "abxyz"), std::strong_ordering::less);
  CHECK_EQ(cmp_tw_ci("abx", "abcyz"), std::strong_ordering::greater);
  CHECK_EQ(cmp_tw_ci("abx", "ABCYZ"), std::strong_ordering::greater);
  CHECK_EQ(cmp_tw_ci("ABX", "abcyz"), std::strong_ordering::greater);
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
