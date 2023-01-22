//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_MIDDLEWARE_T_HPP
#define FITORIA_WEB_MIDDLEWARE_T_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/fwd.hpp>
#include <fitoria/core/net.hpp>

#include <fitoria/web/basic_middleware.hpp>
#include <fitoria/web/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

using middleware_t
    = basic_middleware<http_context&, net::awaitable<http_response>>;

FITORIA_NAMESPACE_END

#include <fitoria/web/http_context.hpp>

#endif