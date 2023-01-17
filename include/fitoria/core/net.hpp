//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#if defined(FITORIA_CXX_COMPILER_MSVC)
// boost/asio/buffer.hpp(247): warning C4702: unreachable code
#pragma warning(push)
#pragma warning(disable : 4702)
#endif

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#if defined(FITORIA_HAS_OPENSSL)
#include <boost/asio/ssl.hpp>
#include <boost/beast/ssl.hpp>
#endif

#if defined(FITORIA_CXX_COMPILER_MSVC)
// boost/asio/buffer.hpp(247): warning C4702: unreachable code
#pragma warning(pop)
#endif

FITORIA_NAMESPACE_BEGIN

namespace net {

using namespace boost::asio;

using boost::beast::error_code;
using boost::beast::system_error;

using boost::beast::flat_buffer;

using boost::beast::async_write;
using boost::beast::get_lowest_layer;

using acceptor = net::as_tuple_t<net::use_awaitable_t<>>::as_default_on_t<
    net::ip::tcp::acceptor>;

using tcp_stream = net::as_tuple_t<net::use_awaitable_t<>>::as_default_on_t<
    boost::beast::tcp_stream>;

using resolver = net::as_tuple_t<net::use_awaitable_t<>>::as_default_on_t<
    net::ip::tcp::resolver>;

using resolver_results = net::ip::basic_resolver_results<net::ip::tcp>;

#if defined(FITORIA_HAS_OPENSSL)
using ssl_stream = boost::beast::ssl_stream<tcp_stream>;
#endif

}

FITORIA_NAMESPACE_END
