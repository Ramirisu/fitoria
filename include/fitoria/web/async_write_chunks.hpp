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
  using boost::beast::http::make_chunk;
  using boost::beast::http::make_chunk_last;

  for (auto data = co_await from.async_read_some(); data;
       data = co_await from.async_read_some()) {
    auto& d = *data;
    if (d) {
      if (auto result = co_await async_write(
              to, make_chunk(net::buffer(*d)), use_awaitable);
          !result) {
        co_return unexpected { result.error() };
      }
    } else {
      co_return unexpected { d.error() };
    }
  }

  co_return co_await async_write(to, make_chunk_last(), use_awaitable);
}

}

FITORIA_NAMESPACE_END

#endif
