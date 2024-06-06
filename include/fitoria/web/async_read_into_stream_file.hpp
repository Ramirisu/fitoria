//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ASYNC_READ_INTO_STREAM_FILE_HPP
#define FITORIA_WEB_ASYNC_READ_INTO_STREAM_FILE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

#include <algorithm>
#include <array>

FITORIA_NAMESPACE_BEGIN

namespace web {

#if defined(BOOST_ASIO_HAS_FILE)

template <async_readable_stream AsyncReadableStream>
auto async_read_into_stream_file(AsyncReadableStream&& stream,
                                 net::stream_file& file)
    -> awaitable<expected<std::size_t, std::error_code>>
{
  std::size_t total = 0;

  for (auto data = co_await stream.async_read_some(); data;
       data = co_await stream.async_read_some()) {
    auto& d = *data;
    if (d) {
      if (auto result
          = co_await net::async_write(file, net::buffer(*d), use_awaitable);
          result) {
        total += *result;
      } else {
        co_return unexpected { result.error() };
      }
    } else {
      co_return unexpected { d.error() };
    }
  }

  co_return total;
}

#endif

}

FITORIA_NAMESPACE_END

#endif
