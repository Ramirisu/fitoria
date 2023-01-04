//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>

#include <chrono>
#include <iostream>
#include <mutex>
#include <source_location>

FITORIA_NAMESPACE_BEGIN

namespace log {

enum class level {
  debug,
  info,
  warning,
  error,
  fatal,
  off,
};

std::string_view to_string(level lv)
{
  switch (lv) {
  case level::debug:
    return "DEBUG";
  case level::info:
    return "INFO";
  case level::warning:
    return "WARNING";
  case level::error:
    return "ERROR";
  case level::fatal:
    return "FATAL";
  default:
    break;
  }

  return "UNKNOWN";
}

class writer {
public:
  virtual ~writer() = default;

  virtual void write(std::string msg) = 0;
};

class stdout_writer : public writer {
public:
  ~stdout_writer() override = default;

  void write(std::string msg) override
  {
    [[maybe_unused]] auto lock = std::scoped_lock(mutex_);
    std::cout << msg;
  }

private:
  std::mutex mutex_;
};

class logger {
public:
  logger(std::shared_ptr<writer> writer)
      : writer_(std::move(writer))
  {
  }

  template <typename Format, typename... Args>
  void
  log(const std::source_location& loc, level lv, Format&& fmt, Args&&... args)
  {
    if (lv < lv_) {
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
    return lv_;
  }

  void set_log_level(level lv) noexcept
  {
    lv_ = lv;
  }

private:
  static std::string_view get_file_name(std::string_view file_path) noexcept
  {
    if (auto pos = file_path.rfind('/'); pos != std::string_view::npos) {
      return file_path.substr(pos + 1);
    }

    return file_path;
  }

  level lv_ = level::off;
  std::shared_ptr<writer> writer_;
};

static std::shared_ptr<logger>& global_logger()
{
  static auto instance
      = std::make_shared<logger>(std::make_shared<stdout_writer>());
  return instance;
}

template <typename... Args>
struct debug {
  debug(Args&&... args,
        const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::debug, std::forward<Args>(args)...);
  }
};

template <typename... Args>
debug(Args&&...) -> debug<Args...>;

template <typename... Args>
struct info {
  info(Args&&... args,
       const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::info, std::forward<Args>(args)...);
  }
};

template <typename... Args>
info(Args&&...) -> info<Args...>;

template <typename... Args>
struct warning {
  warning(Args&&... args,
          const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::warning, std::forward<Args>(args)...);
  }
};

template <typename... Args>
warning(Args&&...) -> warning<Args...>;

template <typename... Args>
struct error {
  error(Args&&... args,
        const std::source_location& loc = std::source_location::current())
  {
    global_logger()->log(loc, level::error, std::forward<Args>(args)...);
  }
};

template <typename... Args>
error(Args&&...) -> error<Args...>;

template <typename... Args>
struct fatal {
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
