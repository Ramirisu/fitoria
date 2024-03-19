//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_SYNC_STREAM_FILE_WRITER_HPP
#define FITORIA_LOG_SYNC_STREAM_FILE_WRITER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/log/async_writer.hpp>

#include <fstream>

FITORIA_NAMESPACE_BEGIN

namespace log {

class sync_stream_file_writer : public async_writer {
  std::ofstream file_;

public:
  sync_stream_file_writer(std::ofstream file)
      : file_(std::move(file))
  {
  }

  ~sync_stream_file_writer() override = default;

  auto async_write(record_ptr rec) -> net::awaitable<void> override
  {
    file_ << format(rec, false);

    co_return;
  }
};

inline std::shared_ptr<async_writer>
make_sync_stream_file_writer(const std::string& path)
{
  return std::make_shared<sync_stream_file_writer>(
      std::ofstream(path, std::ios::binary | std::ios::app));
}

}

FITORIA_NAMESPACE_END

#endif
