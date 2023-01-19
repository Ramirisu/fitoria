//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_SCOPE_HPP
#define FITORIA_WEB_SCOPE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/basic_scope.hpp>
#include <fitoria/web/handler_t.hpp>
#include <fitoria/web/middleware_t.hpp>

FITORIA_NAMESPACE_BEGIN

using scope = basic_scope<middleware_t, handler_t>;

FITORIA_NAMESPACE_END

#endif
