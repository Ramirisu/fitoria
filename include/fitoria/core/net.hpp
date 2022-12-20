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

FITORIA_NAMESPACE_BEGIN

namespace net {

using namespace boost::asio;

namespace errc = boost::beast::errc;

using error_code = boost::beast::error_code;
using boost::beast::flat_buffer;
using tcp_stream = typename boost::beast::tcp_stream::rebind_executor<
    use_awaitable_t<>::executor_with_default<any_io_executor>>::other;

using boost::beast::async_write;
}

namespace http = boost::beast::http;

FITORIA_NAMESPACE_END
