//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ASYNC_READABLE_EOF_STREAM_HPP
#define FITORIA_WEB_ASYNC_READABLE_EOF_STREAM_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/async_readable_stream_concept.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class async_readable_eof_stream {
public:
  using is_async_readable_stream = void;

  async_readable_eof_stream() = default;

  async_readable_eof_stream(const async_readable_eof_stream&) = default;

  async_readable_eof_stream& operator=(const async_readable_eof_stream&)
      = default;

  async_readable_eof_stream(async_readable_eof_stream&&) = default;

  async_readable_eof_stream& operator=(async_readable_eof_stream&&) = default;

  auto size_hint() const noexcept -> optional<std::size_t>
  {
    return 0;
  }

  auto async_read_some(net::mutable_buffer)
      -> net::awaitable<expected<std::size_t, std::error_code>>
  {
    co_return unexpected { make_error_code(net::error::eof) };
  }
};

}

FITORIA_NAMESPACE_END

#endif
