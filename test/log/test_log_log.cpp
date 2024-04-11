//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/log.hpp>

#include <semaphore>

using namespace fitoria::log;

TEST_SUITE_BEGIN("[fitoria.log.log]");

namespace {

class test_writer : public async_writer {
public:
  ~test_writer() override = default;

  auto async_write(record_ptr rec) -> awaitable<void> override
  {
    is_called_ = true;
    output_ += rec->msg;
    sync_.release();
    co_return;
  }

  bool try_acquire()
  {
    return sync_.try_acquire_for(std::chrono::milliseconds(100));
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
  std::binary_semaphore sync_ { 0 };
  bool is_called_ = false;
  std::string output_;
};
}

TEST_CASE("logger with level off")
{
  const char* msg = "hello world";

  registry::global().set_default_logger(
      async_logger::builder().set_filter(filter::at_least(level::off)).build());
  auto writer = std::make_shared<test_writer>();
  registry::global().default_logger()->add_writer(writer);

  trace("{}", msg);
  CHECK(!writer->try_acquire());
  CHECK(!writer->is_called());
  writer->reset();

  debug("{}", msg);
  CHECK(!writer->try_acquire());
  CHECK(!writer->is_called());
  writer->reset();

  info("{}", msg);
  CHECK(!writer->try_acquire());
  CHECK(!writer->is_called());
  writer->reset();

  warning("{}", msg);
  CHECK(!writer->try_acquire());
  CHECK(!writer->is_called());
  writer->reset();

  error("{}", msg);
  CHECK(!writer->try_acquire());
  CHECK(!writer->is_called());
  writer->reset();

  fatal("{}", msg);
  CHECK(!writer->try_acquire());
  CHECK(!writer->is_called());
  writer->reset();
}

TEST_CASE("logger with level info")
{
  const char* msg = "hello world";

  registry::global().set_default_logger(
      async_logger::builder()
          .set_filter(filter::at_least(level::info))
          .build());
  auto writer = std::make_shared<test_writer>();
  registry::global().default_logger()->add_writer(writer);

  trace("{}", msg);
  CHECK(!writer->try_acquire());
  CHECK(!writer->is_called());
  writer->reset();

  debug("{}", msg);
  CHECK(!writer->try_acquire());
  CHECK(!writer->is_called());
  writer->reset();

  info("{}", msg);
  CHECK(writer->try_acquire());
  CHECK(writer->is_called());
  CHECK(writer->contains(msg));
  writer->reset();

  warning("{}", msg);
  CHECK(writer->try_acquire());
  CHECK(writer->is_called());
  CHECK(writer->contains(msg));
  writer->reset();

  error("{}", msg);
  CHECK(writer->try_acquire());
  CHECK(writer->is_called());
  CHECK(writer->contains(msg));
  writer->reset();

  fatal("{}", msg);
  CHECK(writer->try_acquire());
  CHECK(writer->is_called());
  CHECK(writer->contains(msg));
  writer->reset();
}

TEST_SUITE_END();
