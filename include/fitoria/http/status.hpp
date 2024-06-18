//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_HTTP_STATUS_HPP
#define FITORIA_HTTP_STATUS_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/detail/boost.hpp>

FITORIA_NAMESPACE_BEGIN

namespace http {

#if !defined(FITORIA_DOC)

using boost::beast::http::status;

#else

/// @verbatim embed:rst:leading-slashes
///
/// Defines HTTP status code constants.
///
/// DESCRIPTION
///   Defines HTTP status code constants, which are aliases of
///   ``boost::beast::http::status``
///
/// .. seealso::
///
///    `boost::beast::http::status
///    <https://www.boost.org/libs/beast/doc/html/beast/ref/boost__beast__http__status.html>`_
///
/// @endverbatim
enum class status {};

#endif

}

FITORIA_NAMESPACE_END

#endif
