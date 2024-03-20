//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/log.hpp>
#include <fitoria/log/sync_stdout_writer.hpp>

using namespace fitoria::log;

TEST_SUITE_BEGIN("[fitoria.log.sync_stdout_writer]");

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
      std::make_shared<async_logger>(filter::at_least(level::trace)));
  registry::global().default_logger()->add_writer(make_sync_stdout_writer());

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  [[maybe_unused]] auto obj = my_ns::my_class();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

TEST_SUITE_END();
