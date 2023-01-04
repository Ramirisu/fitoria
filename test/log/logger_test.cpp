//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/log/logger.hpp>

using namespace fitoria::log;

TEST_SUITE_BEGIN("log.logger");

namespace {

class test_writer : public writer {
public:
  ~test_writer() override = default;

  void write(std::string msg) override
  {
    is_called_ = true;
    output_ += msg;
  }

  bool is_called() const noexcept
  {
    return is_called_;
  }

  bool contains(std::string_view substr) const noexcept
  {
    return std::string_view(output_).find(substr) != std::string_view::npos;
  }

  void reset()
  {
    is_called_ = false;
    output_.clear();
  }

private:
  bool is_called_ = false;
  std::string output_;
};

}

TEST_CASE("log level test")
{
  const char* msg = "hello world";

  auto writer = std::make_shared<test_writer>();
  global_logger() = std::make_shared<logger>(writer);
  debug("{}", msg);
  CHECK(!writer->is_called());
  info("{}", msg);
  CHECK(!writer->is_called());
  warning("{}", msg);
  CHECK(!writer->is_called());
  error("{}", msg);
  CHECK(!writer->is_called());
  fatal("{}", msg);
  CHECK(!writer->is_called());

  global_logger()->set_log_level(level::warning);
  debug("{}", msg);
  CHECK(!writer->is_called());
  info("{}", msg);
  CHECK(!writer->is_called());
  warning("{}", msg);
  CHECK(writer->contains(msg));
  writer->reset();
  error("{}", msg);
  CHECK(writer->contains(msg));
  writer->reset();
  fatal("{}", msg);
  CHECK(writer->contains(msg));
  writer->reset();
}

TEST_SUITE_END();
