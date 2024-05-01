//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_DETAIL_MAKE_ACCEPTOR_HPP
#define FITORIA_WEB_DETAIL_MAKE_ACCEPTOR_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::detail {

inline auto make_acceptor(const executor_type& ex,
                          net::ip::tcp::endpoint endpoint,
                          int max_listen_connections)
    -> expected<socket_acceptor, std::error_code>
{
  auto acceptor = socket_acceptor(ex);

  boost::system::error_code ec;
  acceptor.open(endpoint.protocol(), ec); // NOLINT
  if (ec) {
    return unexpected { ec };
  }

  acceptor.set_option(net::socket_base::reuse_address(true), ec); // NOLINT
  if (ec) {
    return unexpected { ec };
  }

  acceptor.bind(endpoint, ec); // NOLINT
  if (ec) {
    return unexpected { ec };
  }

  acceptor.listen(max_listen_connections, ec); // NOLINT
  if (ec) {
    return unexpected { ec };
  }

  return acceptor;
}

}

FITORIA_NAMESPACE_END

#endif
