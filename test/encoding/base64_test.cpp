//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/encoding/base64.hpp>

using namespace fitoria::encoding::base64;

TEST_SUITE_BEGIN("encoding.base64");

TEST_CASE("standard_encoder")
{
  struct test_case_t {
    padding pad;
    std::string_view in;
    std::string_view expected;
  };

  const auto test_cases = std::vector<test_case_t> {
    { padding::yes, "", "" },
    { padding::yes, ">?>?>?", "Pj8+Pz4/" },
    { padding::yes, ">?>?>?0", "Pj8+Pz4/MA==" },
    { padding::yes, ">?>?>?01", "Pj8+Pz4/MDE=" },
    { padding::no, "", "" },
    { padding::no, ">?>?>?", "Pj8+Pz4/" },
    { padding::no, ">?>?>?0", "Pj8+Pz4/MA" },
    { padding::no, ">?>?>?01", "Pj8+Pz4/MDE" },
  };

  for (auto& test_case : test_cases) {
    std::string out;
    auto encoder = standard_encoder(test_case.pad);
    encoder.encode(test_case.in.begin(), test_case.in.end(),
                   std::back_inserter(out));
    CHECK_EQ(out, test_case.expected);
  }
}

TEST_CASE("standard_decoder")
{
  struct test_case_t {
    std::string_view in;
    std::string_view expected;
  };

  const auto test_cases = std::vector<test_case_t> {
    { "", "" },
    { "Pj8+Pz4/", ">?>?>?" },
    { "Pj8+Pz4/MA==", ">?>?>?0" },
    { "Pj8+Pz4/MDE=", ">?>?>?01" },
    { "Pj8+Pz4/MA", ">?>?>?0" },
    { "Pj8+Pz4/MDE", ">?>?>?01" },
  };

  for (auto& test_case : test_cases) {
    std::string out;
    auto decoder = standard_decoder();
    decoder.decode(test_case.in.begin(), test_case.in.end(),
                   std::back_inserter(out));
    CHECK_EQ(out, test_case.expected);
  }
}

TEST_CASE("url_encoder")
{
  struct test_case_t {
    padding pad;
    std::string_view in;
    std::string_view expected;
  };

  const auto test_cases = std::vector<test_case_t> {
    { padding::yes, "", "" },
    { padding::yes, ">?>?>?", "Pj8-Pz4_" },
    { padding::yes, ">?>?>?0", "Pj8-Pz4_MA==" },
    { padding::yes, ">?>?>?01", "Pj8-Pz4_MDE=" },
    { padding::no, "", "" },
    { padding::no, ">?>?>?", "Pj8-Pz4_" },
    { padding::no, ">?>?>?0", "Pj8-Pz4_MA" },
    { padding::no, ">?>?>?01", "Pj8-Pz4_MDE" },
  };

  for (auto& test_case : test_cases) {
    std::string out;
    auto encoder = url_encoder(test_case.pad);
    encoder.encode(test_case.in.begin(), test_case.in.end(),
                   std::back_inserter(out));
    CHECK_EQ(out, test_case.expected);
  }
}

TEST_CASE("url_decoder")
{
  struct test_case_t {
    std::string_view in;
    std::string_view expected;
  };

  const auto test_cases = std::vector<test_case_t> {
    { "", "" },
    { "Pj8-Pz4_", ">?>?>?" },
    { "Pj8-Pz4_MA==", ">?>?>?0" },
    { "Pj8-Pz4_MDE=", ">?>?>?01" },
    { "Pj8-Pz4_MA", ">?>?>?0" },
    { "Pj8-Pz4_MDE", ">?>?>?01" },
  };

  for (auto& test_case : test_cases) {
    std::string out;
    auto decoder = url_decoder();
    decoder.decode(test_case.in.begin(), test_case.in.end(),
                   std::back_inserter(out));
    CHECK_EQ(out, test_case.expected);
  }
}

TEST_CASE("standard_decoder & url_decoder with invalid input")
{
  {
    std::string out;
    auto decoder = standard_decoder();
    decoder.decode_next('M', std::back_inserter(out));
    CHECK(!decoder.is_error());
    decoder.decode_next('=', std::back_inserter(out));
    CHECK(decoder.is_error());
    decoder.decode_next('=', std::back_inserter(out));
    CHECK(decoder.is_error());
  }

  const auto test_cases = std::vector<std::vector<std::uint8_t>> {
    // "MA==" -> "0"
    // "MDE=" -> "01"
    // "MDEy" -> "012"
    { 0 },
    { 'M', 0 },
    { 'M', 0, 0 },
    { 'M', 'D', 0 },
    { 'M', 'D', 0, 0 },
    { 'M', 'D', 'E', 0 },
    { 'M', 'D', 'E', 0, 0 },
    { 'M', 'D', 'E', 'y', 0 },
    { 'M', 'D', 'E', 'y', 0, 0 },
    { 'M', 'D', 'E', 'y', '=' },
    { 'M', 'D', 'E', '=', 0 },
    { 'M', 'D', 'E', '=', '=' },
    { 'M', 'A', '=', '=', 0 },
    { 'M', 'A', '=', '=', '=' },
  };

  for (auto& in : test_cases) {
    std::string out;
    auto decoder = standard_decoder();
    CHECK_THROWS_AS(
        decoder.decode(in.begin(), in.end(), std::back_inserter(out)),
        std::invalid_argument);
  }

  for (auto& in : test_cases) {
    std::string out;
    auto decoder = url_decoder();
    CHECK_THROWS_AS(
        decoder.decode(in.begin(), in.end(), std::back_inserter(out)),
        std::invalid_argument);
  }
}

TEST_SUITE_END();
