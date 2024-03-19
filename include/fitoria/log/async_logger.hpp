//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_ASYNC_LOGGER_HPP
#define FITORIA_LOG_ASYNC_LOGGER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/source_location.hpp>

#include <fitoria/log/async_writer.hpp>
#include <fitoria/log/filter.hpp>
#include <fitoria/log/level.hpp>
#include <fitoria/log/record.hpp>

#include <memory>
#include <vector>

FITORIA_NAMESPACE_BEGIN

namespace log {

class async_logger {
  using channel_t = net::as_tuple_t<net::use_awaitable_t<>>::as_default_on_t<
      net::experimental::concurrent_channel<void(net::error_code, record_ptr)>>;

  std::vector<std::shared_ptr<async_writer>> writers_;
  filter filter_;
  channel_t channel_;

  auto async_write_to_all(record_ptr rec) -> net::awaitable<void>
  {
    for (auto& writer : writers_) {
      co_await writer->async_write(rec);
    }
  }

  auto async_dequeue_and_write() -> net::awaitable<void>
  {
    for (;;) {
      auto [ec, rec] = co_await channel_.async_receive();
      if (ec) {
        break;
      }

      co_await async_write_to_all(std::move(rec));
    }
  }

  auto async_enqueue(record_ptr rec) -> net::awaitable<void>
  {
    [[maybe_unused]] auto [ec]
        = co_await channel_.async_send(net::error_code(), rec);
  }

  void log(record::clock_t::time_point time,
           source_location loc,
           level lv,
           std::string msg)
  {
    net::co_spawn(
        net::system_executor(),
        async_enqueue(std::make_shared<record>(time, loc, lv, std::move(msg))),
        net::detached);
  }

public:
  async_logger(filter f = filter::all())
      : filter_(f)
      , channel_(net::system_executor())
  {
    net::co_spawn(
        net::system_executor(), async_dequeue_and_write(), net::detached);
  }

  void add_writer(std::shared_ptr<async_writer> writer)
  {
    writers_.push_back(std::move(writer));
  }

  template <typename... Args>
  void log(source_location loc,
           level lv,
           fmt::format_string<Args...> fmt,
           Args&&... args)
  {
    if (filter_.is_allowed(lv)) {
      log(record::clock_t::now(),
          loc,
          lv,
          fmt::format(fmt, std::forward<Args>(args)...));
    }
  }
};

}

FITORIA_NAMESPACE_END

#endif
