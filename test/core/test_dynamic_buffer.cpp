//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/core/dynamic_buffer.hpp>

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

TEST_CASE("dynamic_buffer")
{
  auto buffer = dynamic_buffer<std::string>();
  CHECK_EQ(buffer.size(), 0);
  CHECK_EQ(buffer.max_size(), std::string().max_size());
  CHECK_EQ(buffer.size(), 0);
  CHECK_EQ(buffer.cdata().size(), 0);

  auto writable = buffer.prepare(1024);
  CHECK_EQ(writable.size(), 1024);
  std::memset(writable.data(), 'x', 256);
  buffer.commit(256);
  CHECK_EQ(buffer.size(), 256);
  CHECK_EQ(buffer.data().size(), 256);
  CHECK_EQ(buffer.cdata().size(), 256);

  buffer.consume(128);
  CHECK_EQ(buffer.size(), 128);
  CHECK_EQ(buffer.data().size(), 128);
  CHECK_EQ(buffer.cdata().size(), 128);

  auto s = buffer.release();
  CHECK_EQ(s.size(), 128);
  CHECK_EQ(s, std::string(128, 'x'));
}

TEST_SUITE_END();
