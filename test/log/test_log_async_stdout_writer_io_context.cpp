//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#define FITORIA_USE_IO_CONTEXT_EXECUTOR
#if defined(FITORIA_HAS_LIBURING)
#define BOOST_ASIO_HAS_IO_URING
#endif
#include <fitoria/log.hpp>
#include <fitoria/log/async_stdout_writer.hpp>

#include <fitoria/log/formatter.hpp>

using namespace fitoria::log;

TEST_SUITE_BEGIN("[fitoria.log.async_stdout_writer.io_context]");

#if defined(FITORIA_HAS_LIBURING)

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
      async_logger::builder()
          .set_filter(filter::at_least(level::trace))
          .build());
  registry::global().default_logger()->add_writer(make_async_stdout_writer());

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  [[maybe_unused]] auto obj = my_ns::my_class();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

#endif

TEST_SUITE_END();
