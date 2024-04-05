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

#include <algorithm>
#include <array>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Container, async_readable_stream AsyncReadableStream>
auto async_read_until_eof(AsyncReadableStream&& stream)
    -> awaitable<expected<Container, std::error_code>>
{
  Container container;

  if (auto size = stream.size_hint(); size) {
    container.reserve(*size);
  }

  const std::size_t bufsize = 4096;
  auto offset = container.size();
  container.resize(offset + bufsize);
  auto size = co_await stream.async_read_some(
      net::buffer(container.data() + offset, bufsize));
  if (!size) {
    co_return unexpected { size.error() };
  }

  while (size) {
    container.resize(offset + *size);

    offset = container.size();
    container.resize(offset + bufsize);
    size = co_await stream.async_read_some(
        net::buffer(container.data() + offset, bufsize));
  }

  if (size.error() != make_error_code(net::error::eof)) {
    co_return unexpected { size.error() };
  }
  container.resize(offset);

  co_return container;
}

}

FITORIA_NAMESPACE_END

#endif
