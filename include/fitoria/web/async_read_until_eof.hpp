//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ASYNC_READ_UNTIL_EOF_HPP
#define FITORIA_WEB_ASYNC_READ_UNTIL_EOF_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/dynamic_buffer.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Container, async_readable_stream AsyncReadableStream>
auto async_read_until_eof(AsyncReadableStream&& stream)
    -> awaitable<expected<Container, std::error_code>>
{
  dynamic_buffer<Container> buffer;

  for (auto data = co_await stream.async_read_some(); data;
       data = co_await stream.async_read_some()) {
    auto& d = *data;
    if (d) {
      auto writable = buffer.prepare(d->size());
      std::memcpy(writable.data(), d->data(), d->size());
      buffer.commit(d->size());
    } else {
      co_return unexpected { d.error() };
    }
  }

  co_return buffer.release();
}

}

FITORIA_NAMESPACE_END

#endif
