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

#include <fitoria/log/async_formattable_writer.hpp>

FITORIA_NAMESPACE_BEGIN

namespace log {

#if defined(BOOST_ASIO_HAS_FILE)

class basic_async_stream_file_writer : public async_formattable_writer {
  net::stream_file file_;

public:
  basic_async_stream_file_writer(formatter fmter, net::stream_file file)
      : async_formattable_writer(fmter)
      , file_(std::move(file))
  {
  }

  ~basic_async_stream_file_writer() override = default;

  auto async_write(record_ptr rec) -> awaitable<void> override
  {
    auto output = this->fmter_.format(rec);

    [[maybe_unused]] auto bytes_written
        = co_await net::async_write(file_, net::buffer(output), use_awaitable);
  }
};

#endif

}

FITORIA_NAMESPACE_END

#endif
