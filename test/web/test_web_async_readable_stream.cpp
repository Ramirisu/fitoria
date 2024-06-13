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

#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/async_readable_file_stream.hpp>
#include <fitoria/web/async_readable_stream_concept.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>

#include <fstream>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.async_readable_stream]");

TEST_CASE("async_readable_vector_stream: read empty")
{
  sync_wait([&]() -> awaitable<void> {
    auto stream = async_readable_vector_stream();
    CHECK(!(co_await stream.async_read_some()));
    CHECK(!(co_await stream.async_read_some()));
  });
}

TEST_CASE("async_readable_vector_stream: read chunk by chunk")
{
  sync_wait([&]() -> awaitable<void> {
    auto stream = async_readable_vector_stream(bytes(9, std::byte(0x40)));
    CHECK_EQ(co_await stream.async_read_some(), bytes(9, std::byte(0x40)));
    CHECK(!(co_await stream.async_read_some()));
  });
}

#if defined(BOOST_ASIO_HAS_FILE)

TEST_CASE("async_readable_file_stream")
{
  const auto file_path = get_temp_file_path();
  const auto data = std::string(1048576, 'a');
  {
    std::ofstream(file_path, std::ios::binary) << data;
  }

  sync_wait([&]() -> awaitable<void> {
    CHECK_EQ(
        co_await async_read_until_eof<std::string>(async_readable_file_stream(
            net::stream_file(co_await net::this_coro::executor,
                             file_path,
                             net::file_base::read_only))),
        data);
  });
}

#endif

TEST_SUITE_END();
