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

  explicit connect_info(
      const net::basic_stream_socket<net::ip::tcp, executor_type>& socket)
      : local_(socket.local_endpoint().address().to_string())
      , remote_(socket.remote_endpoint().address().to_string())
  {
  }

  explicit connect_info(
      const net::basic_stream_socket<net::local::stream_protocol,
                                     executor_type>& socket)
      : local_(socket.local_endpoint().path())
      , remote_(socket.remote_endpoint().path())
  {
  }

  explicit connect_info(const test_stream&)
      : local_("localhost")
      , remote_("localhost")
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
