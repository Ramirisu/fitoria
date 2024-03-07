//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_LOGGER_HPP
#define FITORIA_LOG_LOGGER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>
#include <fitoria/core/source_location.hpp>

#include <fitoria/log/level.hpp>
#include <fitoria/log/writer.hpp>

#include <atomic>
#include <chrono>

FITORIA_NAMESPACE_BEGIN

namespace log {

class logger {
public:
  logger(std::shared_ptr<writer> writer)
      : writer_(std::move(writer))
  {
    if (auto lv_str = std::getenv("CPP_LOG"); lv_str != nullptr) {
      lv_ = to_level(lv_str);
    }
  }

  template <typename Format, typename... Args>
  void log([[maybe_unused]] const source_location& loc,
           level lv,
           Format&& fmt,
           Args&&... args)
  {
    if (!writer_ || lv < log_level()) {
      return;
    }

    std::string msg = fmt::format("{:%FT%TZ} {} ",
                                  std::chrono::floor<std::chrono::seconds>(
                                      std::chrono::system_clock::now()),
                                  to_string(lv));
    msg += fmt::vformat(std::forward<Format>(fmt),
                        fmt::make_format_args(args...));

#if defined(__cpp_lib_source_location)
    msg += fmt::format(" [{}:{}:{}]\n",
                       get_file_name(loc.file_name()),
                       loc.line(),
                       loc.column());
#endif

    writer_->write(std::move(msg));
  }

  level log_level() const noexcept
  {
    return lv_.load(std::memory_order_relaxed); // don't care
  }

  void set_log_level(level lv) noexcept
  {
    lv_.store(lv, std::memory_order_relaxed); // don't care
  }

private:
  static std::string_view get_file_name(std::string_view file_path) noexcept
  {
    if (auto pos = file_path.find_last_of("\\/");
        pos != std::string_view::npos) {
      return file_path.substr(pos + 1);
    }

    return file_path;
  }

  std::atomic<level> lv_ = level::off;
  std::shared_ptr<writer> writer_;
};

static std::shared_ptr<logger>& global_logger()
{
  static auto instance = std::make_shared<logger>(std::shared_ptr<writer>());
  return instance;
}

}

FITORIA_NAMESPACE_END

#endif
