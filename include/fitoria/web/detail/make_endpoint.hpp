//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_DETAIL_MAKE_ENDPOINT_HPP
#define FITORIA_WEB_DETAIL_MAKE_ENDPOINT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::detail {

/// @verbatim embed:rst:leading-slashes
///
/// Create a TCP/IP endpoint by IP and port.
///
/// @endverbatim
inline auto make_endpoint(std::string_view ip_addr, std::uint16_t port)
    -> expected<net::ip::tcp::endpoint, std::error_code>
{
  boost::system::error_code ec;
  auto address = net::ip::make_address(ip_addr, ec);
  if (ec) {
    return unexpected { ec };
  }

  return net::ip::tcp::endpoint(address, port);
}

/// @verbatim embed:rst:leading-slashes
///
/// Create an unix domain socket endpoint by file path.
///
/// @endverbatim
inline auto make_endpoint(std::string_view file_path)
    -> expected<net::local::stream_protocol::endpoint, std::error_code>
{
  return net::local::stream_protocol::endpoint(file_path);
}

}

FITORIA_NAMESPACE_END

#endif
