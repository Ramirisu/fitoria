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

#include <fitoria/log/async_formattable_writer.hpp>

#include <iostream>

FITORIA_NAMESPACE_BEGIN

namespace log {

class sync_stdout_writer : public async_formattable_writer {
  std::ostream& file_;

public:
  sync_stdout_writer()
      : async_formattable_writer(
          formatter::builder().set_color_level_style().build())
      , file_(std::cout)
  {
  }

  ~sync_stdout_writer() override = default;

  auto async_write(record_ptr rec) -> awaitable<void> override
  {
    file_ << this->fmter_.format(rec);

    co_return;
  }
};

inline auto make_sync_stdout_writer()
{
  return std::make_shared<sync_stdout_writer>();
}

}

FITORIA_NAMESPACE_END

#endif
