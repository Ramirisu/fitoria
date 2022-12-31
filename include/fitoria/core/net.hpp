//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#if defined(FITORIA_HAS_OPENSSL)
#include <boost/asio/ssl.hpp>
#include <boost/beast/ssl.hpp>
#endif

FITORIA_NAMESPACE_BEGIN

namespace net {

using namespace boost::asio;

using boost::beast::error_code;
using boost::beast::system_error;

using boost::beast::flat_buffer;

using boost::beast::async_write;
using boost::beast::get_lowest_layer;

using accepter = net::as_tuple_t<net::use_awaitable_t<>>::as_default_on_t<
    net::ip::tcp::acceptor>;

using tcp_stream = net::as_tuple_t<net::use_awaitable_t<>>::as_default_on_t<
    boost::beast::tcp_stream>;

#if defined(FITORIA_HAS_OPENSSL)
using ssl_stream = boost::beast::ssl_stream<tcp_stream>;
#endif

}

FITORIA_NAMESPACE_END
