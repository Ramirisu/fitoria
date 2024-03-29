//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ANY_ASYNC_READABLE_STREAM_HPP
#define FITORIA_WEB_ANY_ASYNC_READABLE_STREAM_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class any_async_readable_stream {
  class base {
  public:
    virtual ~base() = default;
    virtual auto size_hint() const noexcept -> optional<std::size_t> = 0;
    virtual auto async_read_next() -> net::awaitable<
        optional<expected<std::vector<std::byte>, std::error_code>>>
        = 0;
  };

  template <typename AsyncReadableStream>
  class derived : public base {
  public:
    derived(AsyncReadableStream stream)
        : stream_(std::move(stream))
    {
    }

    auto size_hint() const noexcept -> optional<std::size_t> override
    {
      return stream_.size_hint();
    }

    auto async_read_next() -> net::awaitable<
        optional<expected<std::vector<std::byte>, std::error_code>>> override
    {
      return stream_.async_read_next();
    }

  private:
    AsyncReadableStream stream_;
  };

public:
  using is_async_readable_stream = void;

  template <async_readable_stream AsyncReadableStream>
  any_async_readable_stream(AsyncReadableStream&& stream)
    requires(!uncvref_same_as<AsyncReadableStream, any_async_readable_stream>)
      : stream_(std::make_shared<derived<std::decay_t<AsyncReadableStream>>>(
          std::forward<AsyncReadableStream>(stream)))
  {
  }

  any_async_readable_stream(const any_async_readable_stream&) = delete;

  any_async_readable_stream& operator=(const any_async_readable_stream&)
      = delete;

  any_async_readable_stream(any_async_readable_stream&&) = default;

  any_async_readable_stream& operator=(any_async_readable_stream&&) = default;

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    return stream_->size_hint();
  }

  auto async_read_next() -> net::awaitable<
      optional<expected<std::vector<std::byte>, std::error_code>>>
  {
    return stream_->async_read_next();
  }

private:
  // TODO:
  // GCC 12 bug?
  // using unique_ptr causes segmentation fault during destruction
  std::shared_ptr<base> stream_;
};

}

FITORIA_NAMESPACE_END

#endif
