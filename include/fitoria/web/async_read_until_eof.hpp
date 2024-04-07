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

#include <fitoria/web/async_readable_stream_concept.hpp>

#include <fitoria/web/detail/dynamic_buffer.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Container, async_readable_stream AsyncReadableStream>
auto async_read_until_eof(AsyncReadableStream&& stream)
    -> awaitable<expected<Container, std::error_code>>
{
  detail::dynamic_buffer<Container> buffer;

  auto get_size_hint = [&]() -> std::size_t {
    if (auto size = stream.size_hint(); size) {
      return *size;
    }
    return 4096;
  };

  auto hint = get_size_hint();
  auto size = co_await stream.async_read_some(buffer.prepare(hint));
  if (!size) {
    co_return unexpected { size.error() };
  }
  while (size) {
    buffer.commit(*size);

    hint = get_size_hint();
    if (hint == 0) {
      break;
    }
    size = co_await stream.async_read_some(buffer.prepare(hint));
  }
  if (!size && size.error() != make_error_code(net::error::eof)) {
    co_return unexpected { size.error() };
  }

  co_return buffer.release();
}

}

FITORIA_NAMESPACE_END

#endif
