//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ASYNC_READABLE_VECTOR_STREAM_HPP
#define FITORIA_WEB_ASYNC_READABLE_VECTOR_STREAM_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/bytes.hpp>
#include <fitoria/core/optional.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

#include <span>

FITORIA_NAMESPACE_BEGIN

namespace web {

class async_readable_vector_stream {
public:
  using is_async_readable_stream = void;

  async_readable_vector_stream() = default;

  async_readable_vector_stream(bytes data)
  {
    if (!data.empty()) {
      data_.emplace(std::move(data));
    }
  }

  template <typename T, std::size_t N>
  async_readable_vector_stream(std::span<T, N> s)
  {
    if (!s.empty()) {
      data_ = bytes(std::as_bytes(s).begin(), std::as_bytes(s).end());
    }
  }

  async_readable_vector_stream(const async_readable_vector_stream&) = default;

  async_readable_vector_stream& operator=(const async_readable_vector_stream&)
      = default;

  async_readable_vector_stream(async_readable_vector_stream&&) = default;

  async_readable_vector_stream& operator=(async_readable_vector_stream&&)
      = default;

  auto
  async_read_some() -> awaitable<optional<expected<bytes, std::error_code>>>
  {
    if (data_) {
      auto data = std::move(*data_);
      data_.reset();
      co_return data;
    }

    co_return nullopt;
  }

private:
  optional<bytes> data_;
};
}

FITORIA_NAMESPACE_END

#endif
