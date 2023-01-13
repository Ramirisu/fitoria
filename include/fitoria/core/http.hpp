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

#include <boost/beast/http.hpp>

#if defined(FITORIA_CXX_COMPILER_MSVC)
// boost/asio/buffer.hpp(247): warning C4702: unreachable code
#pragma warning(pop)
#endif

FITORIA_NAMESPACE_BEGIN

namespace http = boost::beast::http;

FITORIA_NAMESPACE_END
