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

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/any_routable.hpp>
#include <fitoria/web/as_form.hpp>
#include <fitoria/web/async_message_parser_stream.hpp>
#include <fitoria/web/async_read_into_stream_file.hpp>
#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/async_readable_file_stream.hpp>
#include <fitoria/web/async_readable_stream_concept.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/async_write_chunks.hpp>
#include <fitoria/web/connection_info.hpp>
#include <fitoria/web/error.hpp>
#include <fitoria/web/http_fields.hpp>
#include <fitoria/web/http_request.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/http_server.hpp>
#include <fitoria/web/json_of.hpp>
#include <fitoria/web/middleware/decompress.hpp>
#include <fitoria/web/middleware/exception_handler.hpp>
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
