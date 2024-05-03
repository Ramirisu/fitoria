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

inline auto make_endpoint(std::string_view addr, std::uint16_t port)
    -> expected<net::ip::tcp::endpoint, std::error_code>
{
  boost::system::error_code ec;
  auto address = net::ip::make_address(addr, ec);
  if (ec) {
    return unexpected { ec };
  }

  return net::ip::tcp::endpoint(address, port);
}

}

FITORIA_NAMESPACE_END

#endif
