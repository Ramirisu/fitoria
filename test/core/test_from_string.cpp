//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/core/from_string.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("[fitoria.core.from_string]");

TEST_CASE("string")
{
  const auto s = std::string("-123");
  CHECK_EQ(from_string<std::string>(s), s);
}

TEST_CASE("bool")
{
  CHECK_EQ(from_string<bool>("1"), true);
  CHECK_EQ(from_string<bool>("T"), true);
  CHECK_EQ(from_string<bool>("TrUe"), true);
  CHECK_EQ(from_string<bool>("Y"), true);
  CHECK_EQ(from_string<bool>("YeS"), true);

  CHECK_EQ(from_string<bool>("0"), false);
  CHECK_EQ(from_string<bool>("F"), false);
  CHECK_EQ(from_string<bool>("FaLse"), false);
  CHECK_EQ(from_string<bool>("N"), false);
  CHECK_EQ(from_string<bool>("No"), false);

  CHECK(!from_string<bool>(""));
  CHECK(!from_string<bool>("A"));
  CHECK(!from_string<bool>("trues"));
  CHECK(!from_string<bool>("yess"));
}

TEST_CASE("integral")
{
  const auto n = std::string("-123");
  CHECK_EQ(from_string<std::int8_t>(n), std::int8_t(-123));
  CHECK_EQ(from_string<std::int16_t>(n), std::int16_t(-123));
  CHECK_EQ(from_string<std::int32_t>(n), std::int32_t(-123));
  CHECK_EQ(from_string<std::int64_t>(n), std::int64_t(-123));
  CHECK_EQ(from_string<signed char>(n), (signed char)(-123));
  CHECK_EQ(from_string<signed short>(n), (signed short)(-123));
  CHECK_EQ(from_string<signed int>(n), -123);
  CHECK_EQ(from_string<signed long>(n), -123L);
  CHECK_EQ(from_string<signed long long>(n), -123LL);

  CHECK(!from_string<std::int8_t>("128"));
  CHECK(!from_string<std::int16_t>("32768"));
  CHECK(!from_string<std::int32_t>("2147483648"));
  CHECK(!from_string<std::int64_t>("9223372036854775808LL"));

  const auto p = std::string("123");
  CHECK_EQ(from_string<std::uint8_t>(p), std::uint8_t(123));
  CHECK_EQ(from_string<std::uint16_t>(p), std::uint16_t(123));
  CHECK_EQ(from_string<std::uint32_t>(p), std::uint32_t(123));
  CHECK_EQ(from_string<std::uint64_t>(p), std::uint64_t(123));
  CHECK_EQ(from_string<unsigned char>(p), (unsigned char)(123));
  CHECK_EQ(from_string<unsigned short>(p), (unsigned short)(123));
  CHECK_EQ(from_string<unsigned int>(p), 123U);
  CHECK_EQ(from_string<unsigned long>(p), 123UL);
  CHECK_EQ(from_string<unsigned long long>(p), 123ULL);

  CHECK(!from_string<std::uint8_t>("-1"));
  CHECK(!from_string<std::uint16_t>("-1"));
  CHECK(!from_string<std::uint32_t>("-1"));
  CHECK(!from_string<std::uint64_t>("-1"));
}

template <typename T>
bool equal(expected<T, std::error_code> lhs, T rhs)
{
  return lhs && std::fabs(*lhs - rhs) < std::numeric_limits<T>::epsilon();
}

TEST_CASE("floating point")
{
  const auto n = std::string("-123.456");
  CHECK(equal(from_string<float>(n), -123.456f));
  CHECK(equal(from_string<double>(n), -123.456));
  CHECK(equal(from_string<long double>(n), -123.456l));
}

TEST_SUITE_END();
