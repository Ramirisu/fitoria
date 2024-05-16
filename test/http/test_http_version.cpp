//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/http/version.hpp>

using namespace fitoria::http;

TEST_SUITE_BEGIN("[fitoria.http.version]");

TEST_CASE("to_string")
{
  CHECK_EQ(to_string(version::unknown), "unknown");
  CHECK_EQ(to_string(version::v0_9), "0.9");
  CHECK_EQ(to_string(version::v1_0), "1.0");
  CHECK_EQ(to_string(version::v1_1), "1.1");
  CHECK_EQ(to_string(version::v2_0), "2.0");
  CHECK_EQ(to_string(version::v3_0), "3.0");
}

TEST_CASE("format")
{
  CHECK_EQ(fmt::format("{}", version::unknown), "unknown");
  CHECK_EQ(fmt::format("{}", version::v0_9), "0.9");
  CHECK_EQ(fmt::format("{}", version::v1_0), "1.0");
  CHECK_EQ(fmt::format("{}", version::v1_1), "1.1");
  CHECK_EQ(fmt::format("{}", version::v2_0), "2.0");
  CHECK_EQ(fmt::format("{}", version::v3_0), "3.0");
}

TEST_SUITE_END();
