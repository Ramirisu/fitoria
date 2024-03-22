//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/log/formatter.hpp>

using namespace fitoria;
using namespace fitoria::log;

TEST_SUITE_BEGIN("[fitoria.log.formatter]");

TEST_CASE("default")
{
  auto fmter = formatter::builder().build();
  auto str = fmter.format(std::make_shared<record>(
      record::clock_t::now(), source_location::current(), level::debug, "msg"));
  CHECK(str.find("DEBUG") != std::string::npos);
  CHECK(str.find("/test_log_formatter.cpp") == std::string::npos);
  CHECK(str.find("\\test_log_formatter.cpp") == std::string::npos);
  CHECK(str.ends_with(get_newline(newline_style::system)));
}

TEST_CASE("set pattern with different order")
{
  auto fmter
      = formatter::builder()
            .set_pattern("{TIME:}-{LV:}-{MSG:}-{FUNC:}-{FILE:}-{LINE:}-{COL:}")
            .build();
  auto str = fmter.format(std::make_shared<record>(
      record::clock_t::now(), source_location::current(), level::debug, "msg"));
  CHECK(str.find("DEBUG") != std::string::npos);
  CHECK(str.ends_with(get_newline(newline_style::system)));
}

TEST_CASE("set pattern with less replacement fields")
{
  auto fmter
      = formatter::builder().set_pattern("{FUNC:}-{TIME:}-{MSG:}").build();
  auto str = fmter.format(std::make_shared<record>(
      record::clock_t::now(), source_location::current(), level::debug, "msg"));
  CHECK(str.find("DEBUG") == std::string::npos);
  CHECK(str.ends_with(get_newline(newline_style::system)));
}

TEST_CASE("set newline_style")
{
  auto fmter
      = formatter::builder().set_newline_style(newline_style::cr).build();
  auto str = fmter.format(std::make_shared<record>(
      record::clock_t::now(), source_location::current(), level::debug, "msg"));
  CHECK(str.find("DEBUG") != std::string::npos);
  CHECK(str.ends_with(get_newline(newline_style::cr)));
}

TEST_CASE("set file_name_style")
{
  auto fmter = formatter::builder()
                   .set_file_name_style(file_name_style::full_path)
                   .build();
  auto str = fmter.format(std::make_shared<record>(
      record::clock_t::now(), source_location::current(), level::debug, "msg"));
  auto fpos = str.find("test_log_formatter.cpp") - 1;
  CHECK(str.find_first_of("/\\", fpos) == fpos);
  CHECK(str.ends_with(get_newline(newline_style::system)));
}

TEST_SUITE_END();
