//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>

#include <fitoria/log/level.hpp>
#include <fitoria/log/writer.hpp>

#include <atomic>
#include <chrono>
#include <source_location>

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
  void
  log(const std::source_location& loc, level lv, Format&& fmt, Args&&... args)
  {
    if (lv < log_level()) {
      return;
    }

    std::string msg = fmt::format("{:%FT%TZ} {} ",
                                  std::chrono::floor<std::chrono::seconds>(
                                      std::chrono::system_clock::now()),
                                  to_string(lv));
    msg += fmt::vformat(std::forward<Format>(fmt),
                        fmt::make_format_args(std::forward<Args>(args)...));
    msg += fmt::format(" [{}:{}:{}]\n", get_file_name(loc.file_name()),
                       loc.line(), loc.column());

    FITORIA_ASSERT(writer_);
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
  static auto instance
      = std::make_shared<logger>(std::make_shared<stdout_writer>());
  return instance;
}

template <typename... Args>
struct [[maybe_unused]] debug {
  debug(Args&&... args,
        const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::debug, std::forward<Args>(args)...);
  }
};

template <typename... Args>
debug(Args&&...) -> debug<Args...>;

template <typename... Args>
struct [[maybe_unused]] info {
  info(Args&&... args,
       const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::info, std::forward<Args>(args)...);
  }
};

template <typename... Args>
info(Args&&...) -> info<Args...>;

template <typename... Args>
struct [[maybe_unused]] warning {
  warning(Args&&... args,
          const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::warning, std::forward<Args>(args)...);
  }
};

template <typename... Args>
warning(Args&&...) -> warning<Args...>;

template <typename... Args>
struct [[maybe_unused]] error {
  error(Args&&... args,
        const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::error, std::forward<Args>(args)...);
  }
};

template <typename... Args>
error(Args&&...) -> error<Args...>;

template <typename... Args>
struct [[maybe_unused]] fatal {
  fatal(Args&&... args,
        const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::fatal, std::forward<Args>(args)...);
  }
};

template <typename... Args>
fatal(Args&&...) -> fatal<Args...>;

}

FITORIA_NAMESPACE_END
