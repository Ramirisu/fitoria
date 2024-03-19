//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#if defined(FITORIA_TARGET_LINUX)
#define BOOST_ASIO_HAS_IO_URING
#endif
#include <fitoria/log.hpp>
#include <fitoria/log/async_stream_file_writer.hpp>

using namespace fitoria::log;

TEST_SUITE_BEGIN("[fitoria.log.async_stream_file_writer]");

#if !defined(FITORIA_TARGET_MACOS)

namespace my_ns {
class my_class {
public:
  my_class()
  {
    const char* msg = "hello world";
    trace("write to stdout: {}", msg);
    debug("write to stdout: {}", msg);
    info("write to stdout: {}", msg);
    warning("write to stdout: {}", msg);
    error("write to stdout: {}", msg);
    fatal("write to stdout: {}", msg);
  }
};
}

TEST_CASE("write log")
{
  registry::global().set_default_logger(
      std::make_shared<async_logger>(filter::at_least(level::debug)));
  registry::global().default_logger()->add_writer(
      make_async_stream_file_writer("./test_log_async_stream_file_writer.log"));

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  [[maybe_unused]] auto obj = my_ns::my_class();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

#endif

TEST_SUITE_END();
