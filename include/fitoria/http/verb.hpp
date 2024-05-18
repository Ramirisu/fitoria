//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_HTTP_VERB_HPP
#define FITORIA_HTTP_VERB_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>

FITORIA_NAMESPACE_BEGIN

namespace http {

#if !defined(FITORIA_DOC)

using boost::beast::http::verb;

#else

/// @verbatim embed:rst:leading-slashes
///
/// HTTP method
///
/// Description
///     HTTP method, alias of ``boost::beast::http::verb``
///
/// .. seealso::
///    `boost::beast::http::verb
///    <https://www.boost.org/libs/beast/doc/html/beast/ref/boost__beast__http__verb.html>`_
///
/// @endverbatim

enum verb {};

#endif
}

FITORIA_NAMESPACE_END

#endif
