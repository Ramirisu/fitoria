//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

FITORIA_NAMESPACE_BEGIN

namespace net {

using namespace boost::asio;

namespace errc = boost::beast::errc;

using boost::beast::error_code;
using boost::beast::system_error;

using boost::beast::flat_buffer;
using tcp_stream = typename boost::beast::tcp_stream::rebind_executor<
    use_awaitable_t<>::executor_with_default<any_io_executor>>::other;
using ssl_stream = boost::beast::ssl_stream<tcp_stream>;

using boost::beast::async_write;
using boost::beast::get_lowest_layer;

}

FITORIA_NAMESPACE_END