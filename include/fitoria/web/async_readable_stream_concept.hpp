//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ASYNC_READABLE_STREAM_CONCEPT_HPP
#define FITORIA_WEB_ASYNC_READABLE_STREAM_CONCEPT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>

#include <fitoria/web/http/http.hpp>

#include <algorithm>
#include <cstddef>
#include <span>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename T>
concept async_readable_stream = requires(T t) {
  typename std::remove_cvref_t<T>::is_async_readable_stream;
  {
    t.size_hint()
  } -> std::same_as<optional<std::size_t>>;
  {
    t.async_read_next()
  } -> std::same_as<net::awaitable<
      optional<expected<std::vector<std::byte>, std::error_code>>>>;
};

template <typename Container, async_readable_stream AsyncReadableStream>
auto async_read_all_as(AsyncReadableStream&& stream)
    -> net::awaitable<optional<expected<Container, std::error_code>>>
{
  Container container;

  auto next_chunk = co_await stream.async_read_next();
  if (!next_chunk) {
    co_return nullopt;
  }

  for (; next_chunk; next_chunk = co_await stream.async_read_next()) {
    auto& nc = *next_chunk;
    if (!nc) {
      co_return unexpected { nc.error() };
    }
    const auto offset = container.size();
    container.resize(offset + nc->size());
    std::copy(nc->begin(),
              nc->end(),
              std::as_writable_bytes(std::span(container)).begin() + offset);
  }

  co_return container;
}

template <typename AsyncWritableStream,
          async_readable_stream AsyncReadableStream>
auto async_write_each_chunk(AsyncWritableStream&& to,
                            AsyncReadableStream&& from,
                            std::chrono::milliseconds timeout)
    -> net::awaitable<expected<void, std::error_code>>
{
  using boost::beast::http::make_chunk;
  using boost::beast::http::make_chunk_last;
  boost::system::error_code ec;

  for (auto chunk = co_await from.async_read_next(); chunk;
       chunk = co_await from.async_read_next()) {
    if (!*chunk) {
      co_return unexpected { (*chunk).error() };
    }

    boost::beast::get_lowest_layer(to).expires_after(timeout);
    std::tie(ec, std::ignore) = co_await async_write(
        to,
        make_chunk(net::const_buffer((*chunk)->data(), (*chunk)->size())),
        net::use_ta);
    if (ec) {
      co_return unexpected { ec };
    }
  }

  std::tie(ec, std::ignore)
      = co_await async_write(to, make_chunk_last(), net::use_ta);
  if (ec) {
    co_return unexpected { ec };
  }

  co_return expected<void, std::error_code>();
}
}

FITORIA_NAMESPACE_END

#endif
