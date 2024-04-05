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
    -> net::awaitable<expected<Container, std::error_code>>
{
  using value_type = typename Container::value_type;

  Container container;

  if (auto size = stream.size_hint(); size) {
    container.reserve(*size);
  }

  auto buffer = std::array<value_type, 4096>();
  auto size = co_await stream.async_read_some(net::buffer(buffer));
  if (!size) {
    co_return unexpected { size.error() };
  }

  while (size) {
    const auto offset = container.size();
    container.resize(offset + *size);
    std::copy_n(buffer.begin(), *size, std::next(container.begin(), offset));

    size = co_await stream.async_read_some(net::buffer(buffer));
  }

  if (size.error() != make_error_code(net::error::eof)) {
    co_return unexpected { size.error() };
  }

  co_return container;
}

}

FITORIA_NAMESPACE_END

#endif
