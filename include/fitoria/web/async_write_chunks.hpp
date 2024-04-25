//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ASYNC_WRITE_CHUNKS_HPP
#define FITORIA_WEB_ASYNC_WRITE_CHUNKS_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

#include <array>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename AsyncWritableStream,
          async_readable_stream AsyncReadableStream>
auto async_write_chunks(AsyncWritableStream&& to, AsyncReadableStream&& from)
    -> awaitable<expected<void, std::error_code>>
{
  using boost::beast::async_write;
  using boost::beast::get_lowest_layer;
  using boost::beast::http::make_chunk;
  using boost::beast::http::make_chunk_last;

  auto buffer = std::array<std::byte, 4096>();
  auto bytes_read = co_await from.async_read_some(net::buffer(buffer));
  while (bytes_read) {
    auto bytes_written = co_await async_write(
        to, make_chunk(net::buffer(buffer.data(), *bytes_read)), use_awaitable);
    if (!bytes_written) {
      co_return unexpected { bytes_written.error() };
    }

    bytes_read = co_await from.async_read_some(net::buffer(buffer));
  }

  if (bytes_read.error() != make_error_code(net::error::eof)) {
    co_return unexpected { bytes_read.error() };
  }

  co_return co_await async_write(to, make_chunk_last(), use_awaitable);
}

}

FITORIA_NAMESPACE_END

#endif
