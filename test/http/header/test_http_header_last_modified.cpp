//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/http/header/last_modified.hpp>

#include <fitoria/test/utility.hpp>

using namespace fitoria;
using namespace fitoria::http;

TEST_SUITE_BEGIN("[fitoria.http.header.last_modified]");

TEST_CASE("get formatted time")
{
  CHECK_EQ(http::header::last_modified(chrono::utc_clock::time_point()),
           "Thu, 01 Jan 1970 00:00:00 GMT");
}

TEST_SUITE_END();
