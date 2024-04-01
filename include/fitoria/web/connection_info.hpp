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

  connection_info(net::ip::tcp::endpoint local,
                  net::ip::tcp::endpoint remote,
                  net::ip::tcp::endpoint listen)
      : local_(std::move(local))
      , remote_(std::move(remote))
      , listen_(std::move(listen))
  {
  }

  auto local() const -> const net::ip::tcp::endpoint&
  {
    return local_;
  }

  auto remote() const -> const net::ip::tcp::endpoint&
  {
    return remote_;
  }

  auto listen() const -> const net::ip::tcp::endpoint&
  {
    return listen_;
  }

private:
  net::ip::tcp::endpoint local_;
  net::ip::tcp::endpoint remote_;
  net::ip::tcp::endpoint listen_;
};

}

FITORIA_NAMESPACE_END

#endif
