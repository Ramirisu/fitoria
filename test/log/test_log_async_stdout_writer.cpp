//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#if !defined(_WIN32) && !defined(__APPLE__)
#define BOOST_ASIO_HAS_IO_URING
#endif
#include <fitoria/log.hpp>

using namespace fitoria::log;

TEST_SUITE_BEGIN("[fitoria.log.async_stdout_writer]");

TEST_CASE("windows: iocp, linux: io_uring")
{
  const char* msg = "hello world";

  registry::global().set_default_logger(
      std::make_shared<async_logger>(filter::at_least(level::debug)));
  registry::global().default_logger()->add_writer(
      std::make_shared<async_stdout_writer>());
  debug("write to stdout: {}", msg);
}

TEST_SUITE_END();
