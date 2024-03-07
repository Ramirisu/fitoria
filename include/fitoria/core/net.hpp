//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_NET_HPP
#define FITORIA_CORE_NET_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/detail/boost.hpp>

FITORIA_NAMESPACE_BEGIN

namespace net {

using namespace boost::asio;

using boost::beast::error_code;
using boost::beast::system_error;

using boost::beast::flat_buffer;

using boost::beast::get_lowest_layer;

using acceptor
    = as_tuple_t<use_awaitable_t<>>::as_default_on_t<ip::tcp::acceptor>;

using resolver
    = as_tuple_t<use_awaitable_t<>>::as_default_on_t<ip::tcp::resolver>;

using resolver_results = ip::basic_resolver_results<ip::tcp>;

using tcp_stream
    = as_tuple_t<use_awaitable_t<>>::as_default_on_t<boost::beast::tcp_stream>;

#if defined(FITORIA_HAS_OPENSSL)
using ssl_stream = boost::beast::ssl_stream<tcp_stream>;
#endif

template <typename Stream>
auto get_local_endpoint(Stream& stream)
{
  return net::get_lowest_layer(stream).socket().local_endpoint();
}

template <typename Stream>
auto get_remote_endpoint(Stream& stream)
{
  return net::get_lowest_layer(stream).socket().remote_endpoint();
}

template <typename Awaitable>
auto sync_wait(Awaitable&& awaitable)
{
  io_context ioc;
  auto fut = co_spawn(ioc, std::forward<Awaitable>(awaitable), use_future);
  ioc.run();
  return fut.get();
}

}

FITORIA_NAMESPACE_END

#endif
