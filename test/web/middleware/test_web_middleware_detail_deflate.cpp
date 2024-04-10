//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/async_readable_chunk_stream.hpp>
#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/middleware/detail/async_deflate_stream.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;
using fitoria::test::async_readable_chunk_stream;

TEST_SUITE_BEGIN("[fitoria.web.middleware.detail.deflate]");

TEST_CASE("async_inflate_stream: async_read_some")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> {
      0x4b, 0x4c, 0x4a, 0x4e, 0x49, 0x4d, 0x4b, 0xcf, 0xc8, 0xcc, 0xca,
      0xce, 0xc9, 0xcd, 0xcb, 0x2f, 0x28, 0x2c, 0x2a, 0x2e, 0x29, 0x2d,
      0x2b, 0xaf, 0xa8, 0xac, 0x32, 0x30, 0x34, 0x32, 0x36, 0x31, 0x35,
      0x33, 0xb7, 0xb0, 0x74, 0x74, 0x72, 0x76, 0x71, 0x75, 0x73, 0xf7,
      0xf0, 0xf4, 0xf2, 0xf6, 0xf1, 0xf5, 0xf3, 0x0f, 0x08, 0x0c, 0x0a,
      0x0e, 0x09, 0x0d, 0x0b, 0x8f, 0x88, 0x8c, 0x02, 0x00
    };

    auto stream = middleware::detail::async_inflate_stream(
        async_readable_vector_stream(std::span(in.begin(), in.size())));
    auto buffer = std::array<char, 1>();
    auto out = std::string();
    auto size = co_await stream.async_read_some(net::buffer(buffer));
    while (size) {
      out.append(buffer.data(), *size);
      size = co_await stream.async_read_some(net::buffer(buffer));
    }

    CHECK_EQ(size.error(), make_error_code(net::error::eof));
    CHECK_EQ(
        out,
        std::string_view(
            "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
  });
}

TEST_CASE("async_inflate_stream: in > out")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> {
      0x4b, 0x4c, 0x4a, 0x4e, 0x49, 0x4d, 0x4b, 0xcf, 0xc8, 0xcc, 0xca,
      0xce, 0xc9, 0xcd, 0xcb, 0x2f, 0x28, 0x2c, 0x2a, 0x2e, 0x29, 0x2d,
      0x2b, 0xaf, 0xa8, 0xac, 0x32, 0x30, 0x34, 0x32, 0x36, 0x31, 0x35,
      0x33, 0xb7, 0xb0, 0x74, 0x74, 0x72, 0x76, 0x71, 0x75, 0x73, 0xf7,
      0xf0, 0xf4, 0xf2, 0xf6, 0xf1, 0xf5, 0xf3, 0x0f, 0x08, 0x0c, 0x0a,
      0x0e, 0x09, 0x0d, 0x0b, 0x8f, 0x88, 0x8c, 0x02, 0x00
    };

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_inflate_stream(
            async_readable_vector_stream(std::span(in.begin(), in.size()))));
    CHECK_EQ(
        out,
        std::string_view(
            "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
  });
}

TEST_CASE("async_inflate_stream: in < out")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> { 0x4b, 0x4c, 0x1c, 0x05,
                                                0x23, 0x19, 0x00, 0x00 };

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_inflate_stream(
            async_readable_vector_stream(std::span(in.begin(), in.size()))));
    CHECK_EQ(out, std::string(512, 'a'));
  });
}

TEST_CASE("async_inflate_stream: eof stream")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> {};

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_inflate_stream(
            async_readable_vector_stream()));
    CHECK_EQ(out.error(), make_error_code(net::error::eof));
  });
}

TEST_CASE("async_inflate_stream: invalid stream")
{
  sync_wait([]() -> awaitable<void> {
    // RFC-1951: BFINAL 0, BTYPE 11 (reserved)
    const auto in = std::vector<std::uint8_t> { 0x06 };

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_inflate_stream(
            async_readable_vector_stream(std::span(in.begin(), in.size()))));
    CHECK_EQ(out.error(),
             make_error_code(boost::beast::zlib::error::invalid_block_type));
  });
}

TEST_CASE("async_deflate_stream")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::string_view(
        "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_inflate_stream(
            middleware::detail::async_deflate_stream(
                async_readable_vector_stream(
                    std::span(in.begin(), in.size())))));
    CHECK_EQ(out, in);
  });
}

TEST_CASE("async_deflate_stream: eof stream")
{
  sync_wait([]() -> awaitable<void> {
    auto out = co_await async_read_until_eof<std::vector<std::uint8_t>>(
        middleware::detail::async_deflate_stream(
            async_readable_vector_stream()));
    CHECK_EQ(out.error(), make_error_code(net::error::eof));
  });
}

TEST_SUITE_END();
