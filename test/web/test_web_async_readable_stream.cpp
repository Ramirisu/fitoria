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

#include <fitoria/web/async_readable_file_stream.hpp>
#include <fitoria/web/async_readable_stream_concept.hpp>

#include <fstream>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.async_readable_stream]");

#if defined(BOOST_ASIO_HAS_FILE)

TEST_CASE("async_readable_file_stream")
{
  const auto data = std::string(1024 * 1024, 'a');
  {
    std::ofstream("test_web_async_readable_file_stream.txt", std::ios::binary)
        << data;
  }

  net::sync_wait([&]() -> net::awaitable<void> {
    CHECK_EQ(co_await async_read_all_as<std::string>(async_readable_file_stream(
                 net::stream_file(co_await net::this_coro::executor,
                                  "test_web_async_readable_file_stream.txt",
                                  net::file_base::read_only))),
             data);
  }());
}

#endif

TEST_SUITE_END();
