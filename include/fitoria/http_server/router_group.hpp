//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

#include <fitoria/http_server/basic_router_group.hpp>
#include <fitoria/http_server/handler.hpp>
#include <fitoria/http_server/http_context.hpp>
#include <fitoria/http_server/http_request.hpp>
#include <fitoria/http_server/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

using router_group
    = basic_router_group<handler<http_context&, net::awaitable<http_response>>,
                         handler<http_request&, net::awaitable<http_response>>>;

FITORIA_NAMESPACE_END
