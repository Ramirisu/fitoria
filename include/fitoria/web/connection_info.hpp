//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_CONNECTION_INFO_HPP
#define FITORIA_WEB_CONNECTION_INFO_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class connection_info {
public:
  connection_info() = default;

  connection_info(net::ip::address local_addr,
                  std::uint16_t local_port,
                  net::ip::address remote_addr,
                  std::uint16_t remote_port,
                  net::ip::address listen_addr,
                  std::uint16_t listen_port)
      : local_addr_(std::move(local_addr))
      , local_port_(local_port)
      , remote_addr_(std::move(remote_addr))
      , remote_port_(remote_port)
      , listen_addr_(std::move(listen_addr))
      , listen_port_(listen_port)
  {
  }

  const net::ip::address& local_addr() const noexcept
  {
    return local_addr_;
  }

  std::uint16_t local_port() const noexcept
  {
    return local_port_;
  }

  const net::ip::address& remote_addr() const noexcept
  {
    return remote_addr_;
  }

  std::uint16_t remote_port() const noexcept
  {
    return remote_port_;
  }

  const net::ip::address& listen_addr() const noexcept
  {
    return listen_addr_;
  }

  std::uint16_t listen_port() const noexcept
  {
    return listen_port_;
  }

private:
  net::ip::address local_addr_;
  std::uint16_t local_port_;
  net::ip::address remote_addr_;
  std::uint16_t remote_port_;
  net::ip::address listen_addr_;
  std::uint16_t listen_port_;
};

}

FITORIA_NAMESPACE_END

#endif
