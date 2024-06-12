//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/http/header/range.hpp>

#include <fitoria/test/utility.hpp>

using namespace fitoria;
using namespace fitoria::http;

using fitoria::http::header::range;

TEST_SUITE_BEGIN("[fitoria.http.header.range]");

TEST_CASE("parse")
{
  {
    auto rng = range::parse("bytes=-100", 1000);
    CHECK_EQ(rng->unit(), "bytes");
    CHECK_EQ(rng->size(), 1);
    CHECK_EQ(rng->at(0), range::offset_t { 900, 100 });
    CHECK_EQ((*rng)[0], range::offset_t { 900, 100 });
  }
  {
    auto rng = range::parse("bytes=0-", 1000);
    CHECK_EQ(rng->unit(), "bytes");
    CHECK_EQ(rng->size(), 1);
    CHECK_EQ(rng->at(0), range::offset_t { 0, 1000 });
    CHECK_EQ((*rng)[0], range::offset_t { 0, 1000 });
  }
  {
    auto rng = range::parse("bytes=0-99, 200-", 1000);
    CHECK_EQ(rng->unit(), "bytes");
    CHECK_EQ(rng->size(), 2);
    CHECK_EQ(rng->at(0), range::offset_t { 0, 100 });
    CHECK_EQ(rng->at(1), range::offset_t { 200, 800 });
    CHECK_EQ((*rng)[0], range::offset_t { 0, 100 });
    CHECK_EQ((*rng)[1], range::offset_t { 200, 800 });
  }
  {
    auto rng = range::parse("bytes=0-99", 1000);
    CHECK_EQ(rng->unit(), "bytes");
    CHECK_EQ(rng->size(), 1);
    CHECK_EQ((*rng)[0], range::offset_t { 0, 100 });
    CHECK_EQ(rng->at(0), range::offset_t { 0, 100 });
  }
  {
    auto rng = range::parse("bytes=0-99, 200-399, 700-749", 1000);
    CHECK_EQ(rng->unit(), "bytes");
    CHECK_EQ(rng->size(), 3);
    CHECK_EQ(rng->at(0), range::offset_t { 0, 100 });
    CHECK_EQ(rng->at(1), range::offset_t { 200, 200 });
    CHECK_EQ(rng->at(2), range::offset_t { 700, 50 });
    CHECK_EQ((*rng)[0], range::offset_t { 0, 100 });
    CHECK_EQ((*rng)[1], range::offset_t { 200, 200 });
    CHECK_EQ((*rng)[2], range::offset_t { 700, 50 });
  }
  {
    CHECK(!range::parse("", 1000));
    CHECK(!range::parse("bytes", 1000));
    CHECK(!range::parse("bytes=", 1000));
    CHECK(!range::parse("bytes=0", 1000));
    CHECK(!range::parse("bytes=0-,", 1000));
    CHECK(!range::parse("bytes=0-99,", 1000));
    CHECK(!range::parse("bytes=0-99, 200", 1000));
    CHECK(!range::parse("bytes=0;-99", 1000));
    CHECK(!range::parse("bytes=0-;99", 1000));
  }
}

TEST_CASE("iterator")
{
  {
    auto rng = range::parse("bytes=0-99, 200-399, 700-749", 1000);
    CHECK(test::range_equal(*rng,
                            std::vector<range::offset_t> {
                                { 0, 100 },
                                { 200, 200 },
                                { 700, 50 },
                            }));

    const auto& crng = rng;
    CHECK(test::range_equal(*crng,
                            std::vector<range::offset_t> {
                                { 0, 100 },
                                { 200, 200 },
                                { 700, 50 },
                            }));
  }
}

TEST_SUITE_END();
