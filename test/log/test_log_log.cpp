//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/log/log.hpp>

#if !defined(_WIN32)
#include <cstdlib> // _putenv, setenv
#endif

using namespace fitoria::log;

TEST_SUITE_BEGIN("[fitoria.log.log]");

namespace {

void setenv(const char* name, const char* value)
{
#if defined(_WIN32)
  std::string str = name;
  str += "=";
  str += value;
  _putenv(str.c_str());
#else
  ::setenv(name, value, 1);
#endif
}

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

TEST_CASE("default log level")
{
  const char* msg = "hello world";

  auto writer = std::make_shared<test_writer>();
  global_logger() = std::make_shared<logger>(writer);
  debug("{}", msg);
  CHECK(!writer->is_called());
  writer->reset();
  info("{}", msg);
  CHECK(!writer->is_called());
  writer->reset();
  warning("{}", msg);
  CHECK(!writer->is_called());
  writer->reset();
  error("{}", msg);
  CHECK(!writer->is_called());
  writer->reset();
  fatal("{}", msg);
  CHECK(!writer->is_called());
  writer->reset();
}

TEST_CASE("set_log_level")
{
  const char* msg = "hello world";

  auto writer = std::make_shared<test_writer>();
  global_logger() = std::make_shared<logger>(writer);
  global_logger()->set_log_level(level::warning);
  debug("{}", msg);
  CHECK(!writer->is_called());
  writer->reset();
  info("{}", msg);
  CHECK(!writer->is_called());
  writer->reset();
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

TEST_CASE("log level from env")
{
  const char* msg = "hello world";

  setenv("CPP_LOG", "info");

  auto writer = std::make_shared<test_writer>();
  global_logger() = std::make_shared<logger>(writer);
  debug("{}", msg);
  CHECK(!writer->is_called());
  writer->reset();
  info("{}", msg);
  CHECK(writer->contains(msg));
  writer->reset();
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
