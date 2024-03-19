//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_ASYNC_STDOUT_WRITER_HPP
#define FITORIA_LOG_ASYNC_STDOUT_WRITER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/log/async_writer.hpp>

#if !defined(BOOST_ASIO_HAS_FILE)
#include <iostream>
#endif

FITORIA_NAMESPACE_BEGIN

namespace log {

#if defined(BOOST_ASIO_HAS_FILE)

class async_stdout_writer : public async_writer {
  net::stream_file file_;

public:
  async_stdout_writer()
#if defined(_WIN32)
      : file_(
          net::system_executor(), "CONOUT$", net::file_base::flags::write_only)
#else
      : file_(net::system_executor(), STDOUT_FILENO)
#endif
  {
  }

  ~async_stdout_writer() override = default;

  auto async_write(record_ptr rec) -> net::awaitable<void> override
  {
    auto output = format_record(rec);

    co_await net::async_write(file_,
                              net::const_buffer(output.data(), output.size()),
                              net::use_awaitable);
  }
};

#else
class async_stdout_writer : public async_writer {
  std::ostream& file_;

public:
  async_stdout_writer()
      : file_(std::cout)
  {
  }

  ~async_stdout_writer() override = default;

  auto async_write(record_ptr rec) -> net::awaitable<void> override
  {
    file_ << format_record(rec);
    co_return;
  }
};

#endif

}

FITORIA_NAMESPACE_END

#endif
