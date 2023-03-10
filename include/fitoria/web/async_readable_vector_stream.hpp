//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_ASYNC_READABLE_VECTOR_STREAM_HPP
#define FITORIA_WEB_ASYNC_READABLE_VECTOR_STREAM_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

#include <span>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace web {

class async_readable_vector_stream {
public:
  async_readable_vector_stream(std::vector<std::byte> data)
      : data_(std::move(data))
  {
  }

  template <typename T, std::size_t N>
  async_readable_vector_stream(std::span<T, N> s)
      : data_(std::vector<std::byte>(std::as_bytes(s).begin(),
                                     std::as_bytes(s).end()))
  {
  }

  auto is_chunked() const noexcept -> bool
  {
    return false;
  }

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    return data_.transform([](auto& data) { return data.size(); }).value_or(0);
  }

  auto async_read_next()
      -> lazy<optional<expected<std::vector<std::byte>, net::error_code>>>
  {
    co_return data_.and_then(
        [this](auto& data)
            -> optional<expected<std::vector<std::byte>, net::error_code>> {
          auto moved = std::move(data);
          data_.reset();
          return moved;
        });
  }

  static async_readable_vector_stream eof()
  {
    return {};
  }

  static async_readable_vector_stream empty()
  {
    return async_readable_vector_stream(std::vector<std::byte>());
  }

private:
  async_readable_vector_stream() = default;

  optional<std::vector<std::byte>> data_;
};
}

FITORIA_NAMESPACE_END

#endif
