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

#include <memory>

FITORIA_NAMESPACE_BEGIN

namespace web {

class any_async_readable_stream {
  class base {
  public:
    virtual ~base() = default;
    virtual auto async_read_some(net::mutable_buffer)
        -> awaitable<expected<std::size_t, std::error_code>>
        = 0;
  };

  template <typename AsyncReadableStream>
  class derived : public base {
  public:
    derived(AsyncReadableStream stream)
        : stream_(std::move(stream))
    {
    }

    auto async_read_some(net::mutable_buffer buffer)
        -> awaitable<expected<std::size_t, std::error_code>> override
    {
      return stream_.async_read_some(buffer);
    }

  private:
    AsyncReadableStream stream_;
  };

public:
  using is_async_readable_stream = void;

  template <not_decay_to<any_async_readable_stream> AsyncReadableStream>
  any_async_readable_stream(AsyncReadableStream&& stream)
    requires async_readable_stream<AsyncReadableStream>
      : stream_(std::make_unique<derived<std::decay_t<AsyncReadableStream>>>(
          std::forward<AsyncReadableStream>(stream)))
  {
  }

  any_async_readable_stream(const any_async_readable_stream&) = delete;

  any_async_readable_stream& operator=(const any_async_readable_stream&)
      = delete;

  any_async_readable_stream(any_async_readable_stream&&) = default;

  any_async_readable_stream& operator=(any_async_readable_stream&&) = default;

  auto async_read_some(net::mutable_buffer buffer)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    return stream_->async_read_some(buffer);
  }

private:
  std::unique_ptr<base> stream_;
};

}

FITORIA_NAMESPACE_END

#endif
