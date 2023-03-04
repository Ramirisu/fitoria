//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_ASYNC_READABLE_STREAM_CONCEPT_HPP
#define FITORIA_WEB_ASYNC_READABLE_STREAM_CONCEPT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/error.hpp>
#include <fitoria/core/expected.hpp>
#include <fitoria/core/lazy.hpp>
#include <fitoria/core/net.hpp>

#include <fitoria/web/http/http.hpp>

#include <algorithm>
#include <cstddef>
#include <span>

FITORIA_NAMESPACE_BEGIN

namespace web {

// clang-format off
template <typename T>
concept async_readable_stream = requires(T t) {
  { t.is_chunked() } -> std::same_as<bool>;
  { t.size_hint() } -> std::same_as<optional<std::size_t>>;
  { t.async_read_next() } 
    -> std::same_as<lazy<optional<expected<std::vector<std::byte>, net::error_code>>>>;
};
// clang-format on

template <typename Container, async_readable_stream AsyncReadableStream>
auto async_read_all(AsyncReadableStream&& stream)
    -> lazy<optional<expected<Container, net::error_code>>>
{
  Container container;

  auto next_chunk = co_await stream.async_read_next();
  if (!next_chunk) {
    co_return nullopt;
  }

  do {
    if (!next_chunk.value()) {
      co_return unexpected { next_chunk.value().error() };
    }
    const auto offset = container.size();
    container.resize(offset + next_chunk.value()->size());
    std::copy(next_chunk.value()->begin(),
              next_chunk.value()->end(),
              std::as_writable_bytes(std::span(container)).begin() + offset);
    next_chunk = co_await stream.async_read_next();
  } while (next_chunk);

  co_return container;
}

template <typename AsyncWriteStream, async_readable_stream AsyncReadableStream>
auto async_write_each_chunk(AsyncWriteStream&& to,
                            AsyncReadableStream&& from,
                            std::chrono::milliseconds timeout)
    -> lazy<expected<void, net::error_code>>
{
  using boost::beast::http::make_chunk;
  using boost::beast::http::make_chunk_last;
  auto _ = std::ignore;

  net::error_code ec;

  auto chunk = co_await from.async_read_next();
  while (chunk) {
    if (!*chunk) {
      co_return unexpected { (*chunk).error() };
    }
    net::get_lowest_layer(to).expires_after(timeout);
    tie(ec, _) = co_await async_write(
        to, make_chunk(net::const_buffer((*chunk)->data(), (*chunk)->size())));
    if (ec) {
      co_return unexpected { ec };
    }
    chunk = co_await from.async_read_next();
  }
  tie(ec, _) = co_await async_write(to, make_chunk_last());
  if (ec) {
    co_return unexpected { ec };
  }

  co_return expected<void, net::error_code>();
}
}

FITORIA_NAMESPACE_END

#endif
