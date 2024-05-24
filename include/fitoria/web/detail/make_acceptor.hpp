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

#include <fitoria/web/detail/make_endpoint.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::detail {

template <typename Endpoint>
inline auto make_acceptor(const executor_type& ex,
                          Endpoint endpoint,
                          int max_listen_connections)
    -> expected<socket_acceptor<typename Endpoint::protocol_type>,
                std::error_code>
{
  auto acceptor = socket_acceptor<typename Endpoint::protocol_type>(ex);

  boost::system::error_code ec;
  acceptor.open(endpoint.protocol(), ec);
  if (ec) {
    return unexpected { ec };
  }

  // https://github.com/chriskohlhoff/asio/issues/622
  constexpr auto reuse_addr = !std::same_as<typename Endpoint::protocol_type,
                                            net::local::stream_protocol>;
  acceptor.set_option(net::socket_base::reuse_address(reuse_addr), ec);
  if (ec) {
    return unexpected { ec };
  }

  acceptor.bind(endpoint, ec);
  if (ec) {
    return unexpected { ec };
  }

  acceptor.listen(max_listen_connections, ec);
  if (ec) {
    return unexpected { ec };
  }

  return acceptor;
}

}

FITORIA_NAMESPACE_END

#endif
