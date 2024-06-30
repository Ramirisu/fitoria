//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/http/header/date.hpp>

#include <thread>

using namespace fitoria;

using fitoria::http::header::date;

TEST_SUITE_BEGIN("[fitoria.http.header.date]");

TEST_CASE("to_string")
{
  CHECK_EQ(date(chrono::utc_clock::time_point()).to_string(),
           "Thu, 01 Jan 1970 00:00:00 GMT");
}

TEST_CASE("utc_time")
{
  const auto t = chrono::utc_clock::now();
  CHECK_EQ(date(t).utc_time(), t);
}

TEST_CASE("file_time")
{
  const auto t = std::chrono::file_clock::now();
  CHECK_EQ(date(t).file_time(), t);
}

#if defined(FITORIA_HAS_STD_CHRONO_PARSE)

TEST_CASE("parse")
{
  CHECK_EQ(date::parse("Thu, 01 Jan 1970 00:00:00 GMT"),
           date(chrono::utc_clock::time_point()));
  CHECK_EQ(date::parse("01 Jan 1970 00:00:00 GMT"), nullopt);
}

#endif

TEST_CASE("compare")
{
  auto t = chrono::utc_clock::now();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  CHECK(date(t) < date(chrono::utc_clock::now()));
}

TEST_SUITE_END();
