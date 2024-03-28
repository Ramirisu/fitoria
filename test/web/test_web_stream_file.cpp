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

#include <fitoria/web/stream_file.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.stream_file]");

#if defined(BOOST_ASIO_HAS_FILE)

TEST_CASE("stream_file")
{
  net::sync_wait([]() -> net::awaitable<void> {
    CHECK(!(co_await stream_file::async_open_readonly("abcdefghi")));
  });
}

#endif

TEST_SUITE_END();
