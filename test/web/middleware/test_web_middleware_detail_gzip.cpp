//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/async_readable_chunk_stream.hpp>
#include <fitoria/test/http_server_utils.hpp>

#if defined(FITORIA_HAS_ZLIB)

#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/middleware/detail/async_gzip_stream.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.middleware.detail.gzip]");

TEST_CASE("async_gzip_inflate_stream: async_read_some")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> {
      0x1F, 0x8B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x4B, 0x4C,
      0x4A, 0x4E, 0x49, 0x4D, 0x4B, 0xCF, 0xC8, 0xCC, 0xCA, 0xCE, 0xC9, 0xCD,
      0xCB, 0x2F, 0x28, 0x2C, 0x2A, 0x2E, 0x29, 0x2D, 0x2B, 0xAF, 0xA8, 0xAC,
      0x32, 0x30, 0x34, 0x32, 0x36, 0x31, 0x35, 0x33, 0xB7, 0xB0, 0x74, 0x74,
      0x72, 0x76, 0x71, 0x75, 0x73, 0xF7, 0xF0, 0xF4, 0xF2, 0xF6, 0xF1, 0xF5,
      0xF3, 0x0F, 0x08, 0x0C, 0x0A, 0x0E, 0x09, 0x0D, 0x0B, 0x8F, 0x88, 0x8C,
      0x02, 0x00, 0x32, 0xFA, 0xF8, 0x21, 0x3E, 0x00, 0x00, 0x00
    };

    auto stream = middleware::detail::async_gzip_inflate_stream(
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

TEST_CASE("async_gzip_inflate_stream: in > out")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> {
      0x1F, 0x8B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x4B, 0x4C,
      0x4A, 0x4E, 0x49, 0x4D, 0x4B, 0xCF, 0xC8, 0xCC, 0xCA, 0xCE, 0xC9, 0xCD,
      0xCB, 0x2F, 0x28, 0x2C, 0x2A, 0x2E, 0x29, 0x2D, 0x2B, 0xAF, 0xA8, 0xAC,
      0x32, 0x30, 0x34, 0x32, 0x36, 0x31, 0x35, 0x33, 0xB7, 0xB0, 0x74, 0x74,
      0x72, 0x76, 0x71, 0x75, 0x73, 0xF7, 0xF0, 0xF4, 0xF2, 0xF6, 0xF1, 0xF5,
      0xF3, 0x0F, 0x08, 0x0C, 0x0A, 0x0E, 0x09, 0x0D, 0x0B, 0x8F, 0x88, 0x8C,
      0x02, 0x00, 0x32, 0xFA, 0xF8, 0x21, 0x3E, 0x00, 0x00, 0x00
    };

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_gzip_inflate_stream(
            async_readable_vector_stream(std::span(in.begin(), in.size()))));

    CHECK_EQ(
        out,
        std::string_view(
            "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
  });
}

TEST_CASE("async_gzip_inflate_stream: in < out")
{
  sync_wait([]() -> awaitable<void> {
    const auto in
        = std::vector<std::uint8_t> { 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x0a, 0x4b, 0x4c, 0x1c, 0x05,
                                      0x23, 0x19, 0x00, 0x00, 0x51, 0x70, 0x51,
                                      0xf9, 0x00, 0x02, 0x00, 0x00 };

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_gzip_inflate_stream(
            async_readable_vector_stream(std::span(in.begin(), in.size()))));

    CHECK_EQ(out, std::string(512, 'a'));
  });
}

TEST_CASE("async_gzip_inflate_stream: eof stream")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> {};

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_gzip_inflate_stream(
            async_readable_vector_stream()));
    CHECK_EQ(out.error(), make_error_code(net::error::eof));
  });
}

TEST_CASE("async_gzip_inflate_stream: invalid stream")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> { 0x00, 0x01, 0x02, 0x03 };

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_gzip_inflate_stream(
            async_readable_vector_stream(std::span(in.begin(), in.size()))));
    CHECK_EQ(out.error(), make_error_code(boost::beast::zlib::error::general));
  });
}

TEST_CASE("async_gzip_deflate_stream")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::string_view(
        "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_gzip_inflate_stream(
            middleware::detail::async_gzip_deflate_stream(
                async_readable_vector_stream(
                    std::span(in.begin(), in.size())))));
    CHECK_EQ(out, in);
  });
}

TEST_CASE("async_gzip_deflate_stream: eof stream")
{
  sync_wait([]() -> awaitable<void> {
    auto out = co_await async_read_until_eof<std::vector<std::uint8_t>>(
        middleware::detail::async_gzip_deflate_stream(
            async_readable_vector_stream()));
    CHECK_EQ(out.error(), make_error_code(net::error::eof));
  });
}

TEST_SUITE_END();

#endif
