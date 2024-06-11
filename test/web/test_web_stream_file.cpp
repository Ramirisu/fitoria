//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#if defined(FITORIA_HAS_LIBURING)
#define BOOST_ASIO_HAS_IO_URING
#endif

#include <fitoria/test/http_server_utils.hpp>
#include <fitoria/test/utility.hpp>

#include <fitoria/web/stream_file.hpp>

#include <fstream>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.stream_file]");

#if defined(BOOST_ASIO_HAS_FILE)

TEST_CASE("stream_file")
{
  sync_wait([]() -> awaitable<void> {
    const auto file_path = get_random_temp_file_path();
    CHECK(!stream_file::open_readonly(co_await net::this_coro::executor,
                                      file_path));

    {
      auto ofs = std::ofstream(file_path);
      ofs << file_path;
    }
    CHECK(stream_file::open_readonly(co_await net::this_coro::executor,
                                     file_path));
  });
}

#endif

TEST_SUITE_END();
