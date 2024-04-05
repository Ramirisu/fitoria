//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/async_readable_chunk_stream.hpp>
#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;
using fitoria::test::async_readable_chunk_stream;

TEST_SUITE_BEGIN("[fitoria.web.middleware.deflate]");

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
    auto buffer = std::array<char, 4>();
    auto out = std::string();
    auto size = co_await stream.async_read_some(net::buffer(buffer));
    while (size) {
      out.append(buffer.data(), *size);
      size = co_await stream.async_read_some(net::buffer(buffer));
    }
    CHECK_EQ(size.error(), make_error_code(net::error::eof));

    const auto exp = std::string_view(
        "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    CHECK_EQ(out, exp);
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

    const auto exp = std::string_view(
        "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    CHECK_EQ(out, exp);
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

    const auto exp = std::string(512, 'a');
    CHECK_EQ(out, exp);
  });
}

TEST_CASE("async_inflate_stream: eof stream")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> {};

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_inflate_stream(async_readable_eof_stream()));
    CHECK_EQ(out.error(), make_error_code(net::error::eof));
  });
}

TEST_CASE("async_inflate_stream: empty stream")
{
  sync_wait([]() -> awaitable<void> {
    const auto in = std::vector<std::uint8_t> {};

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_inflate_stream(
            async_readable_vector_stream()));
    CHECK_EQ(out.error(),
             make_error_code(boost::beast::zlib::error::stream_error));
  });
}

TEST_CASE("async_inflate_stream: invalid deflate stream")
{
  sync_wait([]() -> awaitable<void> {
    // RFC-1951: BFINAL 0, BTYPE 11 (reserved)
    const auto in = std::vector<std::uint8_t> { 0x06 };

    auto out = co_await async_read_until_eof<std::string>(
        middleware::detail::async_inflate_stream(
            async_readable_vector_stream(std::span(in.begin(), in.size()))));
    CHECK(!out);
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
        middleware::detail::async_deflate_stream(async_readable_eof_stream()));
    CHECK_EQ(out.error(), make_error_code(net::error::eof));
  });
}

TEST_CASE("async_deflate_stream: empty stream")
{
  sync_wait([]() -> awaitable<void> {
    auto out = co_await async_read_until_eof<std::vector<std::uint8_t>>(
        middleware::detail::async_deflate_stream(
            async_readable_vector_stream()));
    CHECK_EQ(out.error(),
             make_error_code(boost::beast::zlib::error::stream_error));
  });
}

TEST_CASE("deflate middleware")
{
  const auto plain = std::string_view(
      "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  const auto compressed = std::vector<std::uint8_t> {
    0x4b, 0x4c, 0x4a, 0x4e, 0x49, 0x4d, 0x4b, 0xcf, 0xc8, 0xcc, 0xca,
    0xce, 0xc9, 0xcd, 0xcb, 0x2f, 0x28, 0x2c, 0x2a, 0x2e, 0x29, 0x2d,
    0x2b, 0xaf, 0xa8, 0xac, 0x32, 0x30, 0x34, 0x32, 0x36, 0x31, 0x35,
    0x33, 0xb7, 0xb0, 0x74, 0x74, 0x72, 0x76, 0x71, 0x75, 0x73, 0xf7,
    0xf0, 0xf4, 0xf2, 0xf6, 0xf1, 0xf5, 0xf3, 0x0f, 0x08, 0x0c, 0x0a,
    0x0e, 0x09, 0x0d, 0x0b, 0x8f, 0x88, 0x8c, 0x02, 0x00
  };

  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/get/{set_identity}/{chunked}/{empty_body}">(
                       [&]( // const http_request& req,
                           const path_info& path_info,
                           const http_fields& fields,
                           std::string body) -> awaitable<http_response> {
                         CHECK(!fields.get(http::field::content_encoding));
                         // TODO: chunk?
                         //  if (req.body().size_hint()) {
                         //    CHECK_EQ(fields.get(http::field::content_length),
                         //             std::to_string(plain.size()));
                         //  } else {
                         //    CHECK_EQ(fields.get(http::field::content_length),
                         //             nullopt);
                         //  }
                         CHECK_EQ(body, plain);

                         auto res = http_response(http::status::ok);
                         if (path_info.get("set_identity") == "true") {
                           res.set_field(
                               http::field::content_encoding,
                               http::fields::content_encoding::identity());
                         }
                         if (path_info.get("empty_body") == "true") {
                         } else {
                           if (path_info.get("chunked") == "true") {
                             res.set_stream(
                                 async_readable_chunk_stream<5>(plain));
                           } else {
                             res.set_body(plain);
                           }
                         }
                         co_return res;
                       })
                       .use(middleware::deflate()))
            .build();

  struct test_case_t {
    bool send_chunked;
    bool identity;
    bool chunked;
  };

  const auto test_cases = std::vector<test_case_t> {
    { true, true, true },   { true, true, false },   { true, false, true },
    { true, false, false }, { false, true, true },   { false, true, false },
    { false, false, true }, { false, false, false },
  };

  for (auto& test_case : test_cases) {
    server.serve_request(
        fmt::format("/get/{}/{}/false", test_case.identity, test_case.chunked),
        http_request(http::verb::get)
            .set_field(http::field::content_encoding,
                       http::fields::content_encoding::deflate())
            .set_field(http::field::accept_encoding,
                       http::fields::content_encoding::deflate())
            .set_stream([&]() -> any_async_readable_stream {
              auto s = std::span(compressed.data(), compressed.size());
              if (test_case.send_chunked) {
                return async_readable_chunk_stream<5>(s);
              }
              return async_readable_vector_stream(s);
            }()),
        [test_case, &plain](auto res) -> awaitable<void> {
          CHECK_EQ(res.status_code(), http::status::ok);
          if (test_case.identity) {
            CHECK_EQ(res.fields().get(http::field::content_encoding),
                     http::fields::content_encoding::identity());
            CHECK_EQ(co_await res.as_string(), plain);
          } else {
            CHECK_EQ(res.fields().get(http::field::content_encoding),
                     http::fields::content_encoding::deflate());
            CHECK_EQ(co_await async_read_until_eof<std::string>(
                         middleware::detail::async_inflate_stream(
                             std::move(res.body()))),
                     plain);
          }
        });
    server.serve_request(
        "/get/false/false/true",
        http_request(http::verb::get)
            .set_field(http::field::content_encoding,
                       http::fields::content_encoding::deflate())
            .set_field(http::field::accept_encoding,
                       http::fields::content_encoding::deflate())
            .set_stream(async_readable_vector_stream(
                std::span(compressed.data(), compressed.size()))),
        [](auto res) -> awaitable<void> {
          CHECK_EQ(res.status_code(), http::status::ok);
          CHECK(!res.fields().get(http::field::content_encoding));
          CHECK(!(co_await res.as_string()));
        });
  }

  ioc.run();
}

TEST_CASE("deflate middleware: header vary")
{
  auto ioc = net::io_context();
  auto server
      = http_server_builder(ioc)
            .serve(
                route::get<"/">([&](std::string body)
                                    -> awaitable<http_response> {
                  auto res
                      = http_response(http::status::ok).set_body("hello world");
                  if (!body.empty()) {
                    res.set_field(http::field::vary, body);
                  }
                  co_return res;
                }).use(middleware::deflate()))
            .build();

  struct test_case_t {
    std::string input;
    std::string expected;
  };

  const auto test_cases = std::vector<test_case_t> {
    { "", "Content-Encoding" },
    { "*", "*" },
    { "User-Agent", "User-Agent, Content-Encoding" },
  };

  for (auto& test_case : test_cases) {
    server.serve_request(
        "/",
        http_request(http::verb::get)
            .set_field(http::field::accept_encoding,
                       http::fields::content_encoding::deflate())
            .set_body(test_case.input),
        [test_case](auto res) -> awaitable<void> {
          CHECK_EQ(res.status_code(), http::status::ok);
          CHECK_EQ(res.fields().get(http::field::vary), test_case.expected);
          co_return;
        });
  }

  ioc.run();
}

TEST_SUITE_END();
