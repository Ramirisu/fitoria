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

#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/async_readable_eof_stream.hpp>
#include <fitoria/web/async_readable_file_stream.hpp>
#include <fitoria/web/async_readable_stream_concept.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>

#include <fstream>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.async_readable_stream]");

TEST_CASE("async_readable_eof_stream")
{
  sync_wait([&]() -> awaitable<void> {
    auto stream = async_readable_eof_stream();
    auto buffer = std::array<std::byte, 4096>();
    CHECK_EQ((co_await stream.async_read_some(net::buffer(buffer))).error(),
             make_error_code(net::error::eof));
    CHECK_EQ((co_await stream.async_read_some(net::buffer(buffer))).error(),
             make_error_code(net::error::eof));
  });
}

TEST_CASE("async_readable_vector_stream: empty")
{
  sync_wait([&]() -> awaitable<void> {
    auto stream = async_readable_vector_stream(
        std::vector<std::byte>(9, std::byte(0x40)));
    auto buffer = std::array<std::byte, 4>();
    CHECK_EQ((co_await stream.async_read_some(net::buffer(buffer))),
             std::size_t(4));
    CHECK_EQ((co_await stream.async_read_some(net::buffer(buffer))),
             std::size_t(4));
    CHECK_EQ((co_await stream.async_read_some(net::buffer(buffer))),
             std::size_t(1));
    CHECK_EQ((co_await stream.async_read_some(net::buffer(buffer))).error(),
             make_error_code(net::error::eof));
    CHECK_EQ((co_await stream.async_read_some(net::buffer(buffer))).error(),
             make_error_code(net::error::eof));
  });
}

TEST_CASE("async_readable_vector_stream")
{
  sync_wait([&]() -> awaitable<void> {
    auto stream = async_readable_vector_stream();
    auto buffer = std::array<std::byte, 4096>();
    CHECK_EQ((co_await stream.async_read_some(net::buffer(buffer))),
             std::size_t(0));
    CHECK_EQ((co_await stream.async_read_some(net::buffer(buffer))).error(),
             make_error_code(net::error::eof));
    CHECK_EQ((co_await stream.async_read_some(net::buffer(buffer))).error(),
             make_error_code(net::error::eof));
  });
}

#if defined(BOOST_ASIO_HAS_FILE)

TEST_CASE("async_readable_file_stream")
{
  const auto data = std::string(1024 * 1024, 'a');
  {
    std::ofstream("test_web_async_readable_file_stream.txt", std::ios::binary)
        << data;
  }

  sync_wait([&]() -> awaitable<void> {
    CHECK_EQ(
        co_await async_read_until_eof<std::string>(async_readable_file_stream(
            net::stream_file(co_await net::this_coro::executor,
                             "test_web_async_readable_file_stream.txt",
                             net::file_base::read_only))),
        data);
  });
}

#endif

TEST_SUITE_END();
