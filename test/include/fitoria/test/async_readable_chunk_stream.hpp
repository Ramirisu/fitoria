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

#include <fitoria/web/async_readable_stream_concept.hpp>

#include <span>

FITORIA_NAMESPACE_BEGIN

namespace test {

template <std::size_t ChunkSize>
class async_readable_chunk_stream {
public:
  using is_async_readable_stream = void;

  async_readable_chunk_stream(std::string_view sv)
      : async_readable_chunk_stream(
            std::as_bytes(std::span(sv.data(), sv.size())))
  {
  }

  async_readable_chunk_stream(bytes vec)
      : data_(std::move(vec))
  {
  }

  template <typename T, std::size_t N>
  async_readable_chunk_stream(std::span<T, N> s)
      : async_readable_chunk_stream(
            bytes(std::as_bytes(s).begin(), std::as_bytes(s).end()))
  {
  }

  auto
  async_read_some() -> awaitable<optional<expected<bytes, std::error_code>>>
  {
    if (offset_ >= data_.size()) {
      co_return nullopt;
    }

    const auto size = std::min(ChunkSize, data_.size() - offset_);
    auto buffer = bytes(size);
    std::memcpy(buffer.data(), data_.data() + offset_, size);
    offset_ += size;

    co_return buffer;
  }

private:
  bytes data_;
  std::size_t offset_ = 0;
};

}

FITORIA_NAMESPACE_END

#endif
