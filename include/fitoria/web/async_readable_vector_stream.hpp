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

#include <fitoria/web/async_readable_stream_concept.hpp>

#include <span>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

class async_readable_vector_stream {
  struct data_t {
    std::size_t offset;
    std::vector<std::byte> buffer;
  };

public:
  using is_async_readable_stream = void;

  async_readable_vector_stream() = default;

  async_readable_vector_stream(std::vector<std::byte> data)
  {
    if (!data.empty()) {
      data_.emplace(data_t { .offset = 0, .buffer = std::move(data) });
    }
  }

  template <typename T, std::size_t N>
  async_readable_vector_stream(std::span<T, N> s)
  {
    auto b = std::as_bytes(s);
    if (!b.empty()) {
      data_.emplace();
      data_->offset = 0;
      data_->buffer = std::vector<std::byte>(b.begin(), b.end());
    }
  }

  async_readable_vector_stream(const async_readable_vector_stream&) = default;

  async_readable_vector_stream& operator=(const async_readable_vector_stream&)
      = default;

  async_readable_vector_stream(async_readable_vector_stream&&) = default;

  async_readable_vector_stream& operator=(async_readable_vector_stream&&)
      = default;

  auto is_sized() const noexcept -> bool
  {
    return true;
  }

  auto async_read_some(net::mutable_buffer buffer)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    if (data_) {
      const auto size
          = std::min(buffer.size(), data_->buffer.size() - data_->offset);
      std::memcpy(buffer.data(), (data_->buffer.data() + data_->offset), size);
      data_->offset += size;
      if (data_->offset == data_->buffer.size()) {
        data_.reset();
      }

      co_return size;
    }

    co_return unexpected { make_error_code(net::error::eof) };
  }

private:
  optional<data_t> data_;
};
}

FITORIA_NAMESPACE_END

#endif
