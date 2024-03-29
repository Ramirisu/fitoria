//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_HPP
#define FITORIA_WEB_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/any_routable.hpp>
#include <fitoria/web/as_form.hpp>
#include <fitoria/web/connection_info.hpp>
#include <fitoria/web/error.hpp>
#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_fields.hpp>
#include <fitoria/web/http_request.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/http_server.hpp>
#include <fitoria/web/json.hpp>
#include <fitoria/web/middleware/deflate.hpp>
#include <fitoria/web/middleware/exception_handler.hpp>
#include <fitoria/web/middleware/gzip.hpp>
#include <fitoria/web/middleware/logger.hpp>
#include <fitoria/web/middleware_concept.hpp>
#include <fitoria/web/path_info.hpp>
#include <fitoria/web/path_of.hpp>
#include <fitoria/web/query_map.hpp>
#include <fitoria/web/query_of.hpp>
#include <fitoria/web/route.hpp>
#include <fitoria/web/router.hpp>
#include <fitoria/web/scope.hpp>
#include <fitoria/web/state_map.hpp>
#include <fitoria/web/state_of.hpp>
#include <fitoria/web/stream_file.hpp>

#endif
