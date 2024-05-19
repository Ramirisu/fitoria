//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/core/dynamic_buffer.hpp>

#include <numeric>
#include <string>
#include <vector>

using namespace fitoria;

TEST_SUITE_BEGIN("[fitoria.core.dynamic_buffer]");

TEST_CASE("is_dynamic_buffer")
{
  static_assert(net::is_dynamic_buffer<dynamic_buffer<std::string>>::value);
  static_assert(
      net::is_dynamic_buffer<dynamic_buffer<std::vector<std::uint8_t>>>::value);
}

TEST_CASE("basic")
{
  auto buffer = dynamic_buffer<std::string>();
  CHECK_EQ(buffer.size(), 0);
  CHECK_EQ(buffer.max_size(), std::string().max_size());
  CHECK_EQ(buffer.size(), 0);
  CHECK_EQ(buffer.cdata().size(), 0);
  {
    auto w = buffer.prepare(4);
    CHECK_EQ(w.size(), 4);
    ((char*)w.data())[0] = 'a';
    ((char*)w.data())[1] = 'b';
    buffer.commit(2);
  }
  {
    auto r = buffer.cdata();
    CHECK_EQ(r.size(), 2);
    CHECK_EQ(((char*)r.data())[0], 'a');
    CHECK_EQ(((char*)r.data())[1], 'b');
    buffer.consume(1);
    CHECK_EQ(buffer.size(), 1);
  }
  {
    auto w = buffer.prepare(4);
    CHECK_EQ(w.size(), 4);
    ((char*)w.data())[0] = 'c';
    ((char*)w.data())[1] = 'd';
    buffer.commit(2);
  }
  {
    auto r = buffer.cdata();
    CHECK_EQ(r.size(), 3);
    CHECK_EQ(((char*)r.data())[0], 'b');
    CHECK_EQ(((char*)r.data())[1], 'c');
    CHECK_EQ(((char*)r.data())[2], 'd');
    buffer.consume(2);
    CHECK_EQ(buffer.size(), 1);
  }
}

TEST_CASE("with limit: required size less than or equal")
{
  auto buffer = dynamic_buffer<std::string>(256);
  buffer.prepare(128);
  buffer.commit(128);
  buffer.consume(64);
  buffer.prepare(192);
  buffer.commit(192);
}

TEST_CASE("with limit: required size too large")
{
  auto buffer = dynamic_buffer<std::string>(256);
  buffer.prepare(128);
  buffer.commit(128);
  CHECK_THROWS_AS(buffer.prepare(129), std::length_error);
}

TEST_SUITE_END();
