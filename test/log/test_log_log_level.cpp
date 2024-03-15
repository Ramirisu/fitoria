//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/log/level.hpp>

using namespace fitoria::log;

TEST_SUITE_BEGIN("[fitoria.log.log_level]");

TEST_CASE("to_string")
{
  CHECK_EQ(to_string(level::debug), "DEBUG");
  CHECK_EQ(to_string(level::info), "INFO");
  CHECK_EQ(to_string(level::warning), "WARNING");
  CHECK_EQ(to_string(level::error), "ERROR");
  CHECK_EQ(to_string(level::fatal), "FATAL");
  CHECK_EQ(to_string(static_cast<level>(12345)), "UNKNOWN");
}

TEST_CASE("to_level")
{
  CHECK_EQ(to_level("dEbUG"), level::debug);
  CHECK_EQ(to_level("iNfO"), level::info);
  CHECK_EQ(to_level("WarnING"), level::warning);
  CHECK_EQ(to_level("ErrOR"), level::error);
  CHECK_EQ(to_level("FaTAL"), level::fatal);
  CHECK_EQ(to_level("ABCDE"), level::off);
}

TEST_SUITE_END();
