//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/log.hpp>

#include <semaphore>

#if !defined(_WIN32)
#include <cstdlib> // _putenv, setenv
#endif

using namespace fitoria::log;

TEST_SUITE_BEGIN("[fitoria.log.filter]");

namespace {

void setenv(const char* name, const char* value)
{
#if defined(_WIN32)
  std::string str = name;
  str += "=";
  str += value;
  _putenv(str.c_str());
#else
  ::setenv(name, value, 1);
#endif
}

}

TEST_CASE("logger with filter()")
{
  auto flt = filter::all();
  CHECK(flt.is_allowed(level::trace));
  CHECK(flt.is_allowed(level::debug));
  CHECK(flt.is_allowed(level::info));
  CHECK(flt.is_allowed(level::warning));
  CHECK(flt.is_allowed(level::error));
  CHECK(flt.is_allowed(level::fatal));
}

TEST_CASE("logger with filter({ debug, error })")
{
  auto flt = filter({ level::debug, level::error });
  CHECK(!flt.is_allowed(level::trace));
  CHECK(flt.is_allowed(level::debug));
  CHECK(!flt.is_allowed(level::info));
  CHECK(!flt.is_allowed(level::warning));
  CHECK(flt.is_allowed(level::error));
  CHECK(!flt.is_allowed(level::fatal));
}

TEST_CASE("logger with filter::at_least()")
{
  auto flt = filter::at_least(level::info);
  CHECK(!flt.is_allowed(level::trace));
  CHECK(!flt.is_allowed(level::debug));
  CHECK(flt.is_allowed(level::info));
  CHECK(flt.is_allowed(level::warning));
  CHECK(flt.is_allowed(level::error));
  CHECK(flt.is_allowed(level::fatal));
}

TEST_CASE("logger with filter::at_least()")
{
  auto flt = filter::at_least(level::off);
  CHECK(!flt.is_allowed(level::trace));
  CHECK(!flt.is_allowed(level::debug));
  CHECK(!flt.is_allowed(level::info));
  CHECK(!flt.is_allowed(level::warning));
  CHECK(!flt.is_allowed(level::error));
  CHECK(!flt.is_allowed(level::fatal));
}

TEST_CASE("logger with filter::from_env()")
{
  setenv("CPP_LOG", "warning");

  auto flt = filter::from_env();
  CHECK(!flt.is_allowed(level::trace));
  CHECK(!flt.is_allowed(level::debug));
  CHECK(!flt.is_allowed(level::info));
  CHECK(flt.is_allowed(level::warning));
  CHECK(flt.is_allowed(level::error));
  CHECK(flt.is_allowed(level::fatal));
}

TEST_SUITE_END();
