//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/async_readable_chunk_stream.hpp>
#include <fitoria/test/http_server_utils.hpp>

#if defined(FITORIA_HAS_BROTLI)

#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/middleware/detail/async_brotli_deflate_stream.hpp>
#include <fitoria/web/middleware/detail/async_brotli_inflate_stream.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;
using fitoria::test::async_readable_chunk_stream;

TEST_SUITE_BEGIN("[fitoria.web.middleware.detail.brotli]");

TEST_CASE("inflate: 1 byte buffer")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> {
      0x8b, 0x1e, 0x80, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
      0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73,
      0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x30, 0x31, 0x32, 0x33,
      0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45,
      0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
      0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x03
    };

    auto stream = middleware::detail::async_brotli_inflate_stream(
        async_readable_chunk_stream<1>(std::span(in.begin(), in.size())));
    auto buffer = dynamic_buffer<std::string>();
    for (auto data = co_await stream.async_read_some(); data;
         data = co_await stream.async_read_some()) {
      auto& d = *data;
      REQUIRE(d);
      auto writable = buffer.prepare(d->size());
      std::memcpy(writable.data(), d->data(), d->size());
      buffer.commit(d->size());
    }

    REQUIRE_EQ(
        buffer.release(),
        std::string_view(
            "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
  });
}

TEST_CASE("inflate: in > out")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> {
      0x8b, 0x1e, 0x80, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
      0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73,
      0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x30, 0x31, 0x32, 0x33,
      0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45,
      0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
      0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x03
    };

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_brotli_inflate_stream(
            async_readable_vector_stream(std::span(in.begin(), in.size()))));

    CHECK_EQ(
        out,
        std::string_view(
            "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
  });
}

TEST_CASE("inflate: in < out")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> { 0x5b, 0xff, 0xff, 0x8f, 0x5f,
                                                0x22, 0x2c, 0x1e, 0x0b, 0x04,
                                                0x72, 0xef, 0x1f, 0x00 };

    CHECK_EQ(co_await async_read_until_eof<std::string>(
                 middleware::detail::async_brotli_inflate_stream(
                     async_readable_vector_stream(
                         std::span(in.begin(), in.size())))),
             std::string(1048576, 'a'));
  });
}

TEST_CASE("inflate: eof stream")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> {};

    auto stream = middleware::detail::async_brotli_inflate_stream(
        async_readable_vector_stream());
    REQUIRE(!(co_await stream.async_read_some()));
  });
}

TEST_CASE("inflate: invalid stream")
{
  sync_wait([]() -> awaitable<void> {
    // RFC 7932
    const auto in = std::vector<std::uint8_t> { 0b0010001 };

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_brotli_inflate_stream(
            async_readable_vector_stream(std::span(in.begin(), in.size()))));
    CHECK_EQ(out.error(),
             make_error_code(middleware::detail::brotli_error::error));
  });
}

TEST_CASE("deflate: valid stream")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::string_view(
        "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_brotli_inflate_stream(
            middleware::detail::async_brotli_deflate_stream(
                async_readable_vector_stream(
                    std::span(in.begin(), in.size())))));
    CHECK_EQ(out, in);
  });
}

TEST_CASE("deflate: valid stream, 1 byte per call")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::string_view(
        "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_brotli_inflate_stream(
            middleware::detail::async_brotli_deflate_stream(
                async_readable_chunk_stream<1>(
                    std::span(in.begin(), in.size())))));
    CHECK_EQ(out, in);
  });
}

TEST_CASE("deflate: eof stream")
{
  sync_wait([]() -> awaitable<void> {
    auto out = co_await async_read_until_eof<std::vector<std::uint8_t>>(
        middleware::detail::async_brotli_deflate_stream(
            async_readable_vector_stream()));
    CHECK_EQ(out, std::vector<std::uint8_t> { 0x3b });
  });
}

TEST_SUITE_END();

#endif
