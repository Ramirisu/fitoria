//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_CONNECT_INFO_HPP
#define FITORIA_WEB_CONNECT_INFO_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class connect_info {
public:
  connect_info() = default;

  connect_info(net::ip::tcp::endpoint local, net::ip::tcp::endpoint remote)
      : local_(local.address().to_string())
      , remote_(remote.address().to_string())
  {
  }

  connect_info(net::local::stream_protocol::endpoint local,
               net::local::stream_protocol::endpoint remote)
      : local_(local.path())
      , remote_(remote.path())
  {
  }

  auto local() const -> const std::string&
  {
    return local_;
  }

  auto remote() const -> const std::string&
  {
    return remote_;
  }

private:
  std::string local_;
  std::string remote_;
};

}

FITORIA_NAMESPACE_END

#endif
