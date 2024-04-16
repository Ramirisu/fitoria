//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_HTTP_BODY_HPP
#define FITORIA_WEB_HTTP_BODY_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>

#include <variant>

FITORIA_NAMESPACE_BEGIN

namespace web {

class http_body {
public:
  struct null { };

  struct sized {
    std::size_t size;
  };

  struct chunked { };

  using size_type = std::variant<null, sized, chunked>;

  http_body()
      : size_(null {})
      , stream_(async_readable_vector_stream())
  {
  }

  http_body(size_type size, any_async_readable_stream stream)
      : size_(std::move(size))
      , stream_(std::move(stream))
  {
  }

  http_body(const http_body&) = delete;

  http_body& operator=(const http_body&) = delete;

  http_body(http_body&&) = default;

  http_body& operator=(http_body&&) = default;

  auto size() noexcept -> size_type&
  {
    return size_;
  }

  auto size() const noexcept -> const size_type&
  {
    return size_;
  }

  auto stream() noexcept -> any_async_readable_stream&
  {
    return stream_;
  }

  auto stream() const noexcept -> const any_async_readable_stream&
  {
    return stream_;
  }

private:
  size_type size_;
  any_async_readable_stream stream_;
};

}

FITORIA_NAMESPACE_END

#endif
