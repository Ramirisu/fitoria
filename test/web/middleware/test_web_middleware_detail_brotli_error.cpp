//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/web/middleware/detail/brotli_error.hpp>

using namespace fitoria;
using namespace fitoria::web::middleware::detail;

TEST_SUITE_BEGIN("[fitoria.web.brotli_error]");

TEST_CASE("message")
{
  CHECK_EQ(make_error_code(brotli_error::error).message(), "decode error");
  CHECK_EQ(make_error_code(brotli_error::need_more_input).message(),
           "need more input");
  CHECK_EQ(make_error_code(brotli_error::need_more_output).message(),
           "need more output");
  CHECK_EQ(make_error_code(brotli_error::init).message(),
           "initialzation error");
  CHECK_EQ(make_error_code(brotli_error::unknown).message(), "unknown error");
  CHECK_EQ(make_error_code(static_cast<brotli_error>(-1)).message(), "");
}

TEST_CASE("category")
{
  CHECK_EQ(make_error_code(brotli_error::error).category().name(),
           std::string_view("fitoria.web.brotli_error"));
}

TEST_SUITE_END();
