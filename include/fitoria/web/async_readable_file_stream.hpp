//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ASYNC_READABLE_FILE_STREAM_HPP
#define FITORIA_WEB_ASYNC_READABLE_FILE_STREAM_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/utility.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

#if defined(BOOST_ASIO_HAS_FILE)

class async_readable_file_stream {
public:
  using is_async_readable_stream = void;

  async_readable_file_stream(net::stream_file file)
      : file_(std::move(file))
  {
  }

  async_readable_file_stream(const async_readable_file_stream&) = delete;

  async_readable_file_stream& operator=(const async_readable_file_stream&)
      = delete;

  async_readable_file_stream(async_readable_file_stream&&) = default;

  async_readable_file_stream& operator=(async_readable_file_stream&&) = default;

  auto async_read_some(net::mutable_buffer buffer)
      -> awaitable<expected<std::size_t, std::error_code>>
  {
    co_return co_await file_.async_read_some(buffer, use_awaitable);
  }

private:
  net::stream_file file_;
};

#endif

}

FITORIA_NAMESPACE_END

#endif
