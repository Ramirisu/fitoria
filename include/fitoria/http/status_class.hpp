//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_HTTP_STATUS_CLASS_HPP
#define FITORIA_HTTP_STATUS_CLASS_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>

FITORIA_NAMESPACE_BEGIN

namespace http {

#if !defined(FITORIA_DOC)

using boost::beast::http::status_class;

#else

/// @verbatim embed:rst:leading-slashes
///
/// HTTP status category
///
/// Description
///     HTTP status category, alias of ``boost::beast::http::status_class``
///
/// .. seealso::
///    `boost::beast::http::status_class
///    <https://www.boost.org/libs/beast/doc/html/beast/ref/boost__beast__http__status_class.html>`_
///
/// @endverbatim
enum status_class {};

#endif

}

FITORIA_NAMESPACE_END

#endif
