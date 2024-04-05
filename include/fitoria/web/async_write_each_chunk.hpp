//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ASYNC_WRITE_EACH_CHUNK_HPP
#define FITORIA_WEB_ASYNC_WRITE_EACH_CHUNK_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

#include <array>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename AsyncWritableStream,
          async_readable_stream AsyncReadableStream>
auto async_write_each_chunk(AsyncWritableStream&& to,
                            AsyncReadableStream&& from,
                            std::chrono::milliseconds timeout)
    -> awaitable<expected<void, std::error_code>>
{
  using boost::beast::async_write;
  using boost::beast::get_lowest_layer;
  using boost::beast::http::make_chunk;
  using boost::beast::http::make_chunk_last;

  boost::system::error_code ec;

  auto buffer = std::array<std::byte, 4096>();
  auto size = co_await from.async_read_some(net::buffer(buffer));
  while (size) {
    get_lowest_layer(to).expires_after(timeout);
    std::tie(ec, std::ignore) = co_await async_write(
        to, make_chunk(net::buffer(buffer.data(), *size)), use_awaitable);
    if (ec) {
      co_return unexpected { ec };
    }

    size = co_await from.async_read_some(net::buffer(buffer));
  }
  if (size.error() != make_error_code(net::error::eof)) {
    co_return unexpected { size.error() };
  }

  get_lowest_layer(to).expires_after(timeout);
  std::tie(ec, std::ignore)
      = co_await async_write(to, make_chunk_last(), use_awaitable);
  if (ec) {
    co_return unexpected { ec };
  }

  co_return expected<void, std::error_code>();
}

}

FITORIA_NAMESPACE_END

#endif
