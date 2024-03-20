//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_SYNC_STDOUT_WRITER_HPP
#define FITORIA_LOG_SYNC_STDOUT_WRITER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/log/detail/format.hpp>

#include <fitoria/log/async_writer.hpp>

#include <iostream>

FITORIA_NAMESPACE_BEGIN

namespace log {

class sync_stdout_writer : public async_writer {
  std::ostream& file_;

public:
  sync_stdout_writer()
      : file_(std::cout)
  {
  }

  ~sync_stdout_writer() override = default;

  auto async_write(record_ptr rec) -> net::awaitable<void> override
  {
    file_ << detail::format(rec, true);

    co_return;
  }
};

inline std::shared_ptr<async_writer> make_sync_stdout_writer()
{
  return std::make_shared<sync_stdout_writer>();
}

}

FITORIA_NAMESPACE_END

#endif
