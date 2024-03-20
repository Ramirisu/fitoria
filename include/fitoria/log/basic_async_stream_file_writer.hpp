//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_BASIC_ASYNC_STREAM_FILE_WRITER_HPP
#define FITORIA_LOG_BASIC_ASYNC_STREAM_FILE_WRITER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/log/detail/format.hpp>

#include <fitoria/log/async_writer.hpp>

FITORIA_NAMESPACE_BEGIN

namespace log {

#if defined(BOOST_ASIO_HAS_FILE)

template <bool Colorful>
class basic_async_stream_file_writer : public async_writer {
  net::stream_file file_;

public:
  basic_async_stream_file_writer(net::stream_file file)
      : file_(std::move(file))
  {
  }

  ~basic_async_stream_file_writer() override = default;

  auto async_write(record_ptr rec) -> net::awaitable<void> override
  {
    auto output = detail::format(rec, Colorful);

    co_await net::async_write(file_,
                              net::const_buffer(output.data(), output.size()),
                              net::use_awaitable);
  }
};

#endif

}

FITORIA_NAMESPACE_END

#endif
