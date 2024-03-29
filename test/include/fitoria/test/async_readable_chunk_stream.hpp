//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_TEST_ASYNC_READABLE_CHUNK_STREAM_HPP
#define FITORIA_TEST_ASYNC_READABLE_CHUNK_STREAM_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/optional.hpp>

#include <algorithm>
#include <cstddef>
#include <span>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace test {

template <std::size_t ChunkSize>
class async_readable_chunk_stream {
public:
  using is_async_readable_stream = void;

  async_readable_chunk_stream(std::string_view sv)
      : data_(std::as_bytes(std::span(sv.data(), sv.size())))
  {
  }

  template <typename T, std::size_t N>
  async_readable_chunk_stream(std::span<T, N> s)
      : data_(std::as_bytes(s))
  {
  }

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    // return `nullopt` here to tell serializer to do chunked transfer encoding
    // even though we know the exact size of the stream is `data_.size()`.
    return nullopt;
  }

  auto async_read_next() -> net::awaitable<
      optional<expected<std::vector<std::byte>, std::error_code>>>
  {
    if (written >= data_.size()) {
      co_return nullopt;
    }

    const auto chunk_size = std::min(ChunkSize, data_.size() - written);
    auto sub = data_.subspan(written, chunk_size);
    written += chunk_size;
    co_return std::vector<std::byte>(sub.begin(), sub.end());
  }

private:
  std::span<const std::byte> data_;
  std::size_t written = 0;
};

}

FITORIA_NAMESPACE_END

#endif
