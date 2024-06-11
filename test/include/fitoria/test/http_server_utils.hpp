//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_TEST_HTTP_SERVER_UTILS_HPP
#define FITORIA_TEST_HTTP_SERVER_UTILS_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/url.hpp>

FITORIA_NAMESPACE_BEGIN

namespace test {

inline const auto server_start_wait_time = std::chrono::milliseconds(100);
inline const char* localhost = "127.0.0.1";
inline std::uint16_t generate_port()
{
  static std::uint16_t port = 43210;
  return ++port;
}

inline std::string to_local_url(boost::urls::scheme scheme,
                                std::uint16_t port,
                                std::string_view path)
{
  auto scheme_str = std::string_view(to_string(scheme));
  return fmt::format("{}://{}:{}{}", scheme_str, localhost, port, path);
}

template <typename Awaitable>
void sync_wait(Awaitable&& awaitable)
{
  net::io_context ioc;
  auto fut = co_spawn(ioc, std::forward<Awaitable>(awaitable), net::use_future);
  ioc.run();
  fut.get();
}

}

FITORIA_NAMESPACE_END

#endif
