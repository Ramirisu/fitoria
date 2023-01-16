//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>
#include <fitoria/core/utility.hpp>

#include <type_traits>

FITORIA_NAMESPACE_BEGIN

namespace http {

using boost::beast::http::field;
using boost::beast::http::status;
using boost::beast::http::status_class;
using boost::beast::http::verb;

using boost::beast::http::async_read;
using boost::beast::http::async_read_header;
using boost::beast::http::async_write;
using boost::beast::http::async_write_header;
using boost::beast::http::error;
using boost::beast::http::fields;
using boost::beast::http::message_generator;
using boost::beast::http::request;
using boost::beast::http::request_parser;
using boost::beast::http::request_serializer;
using boost::beast::http::response;
using boost::beast::http::string_body;
using boost::beast::http::to_status_class;
}

FITORIA_NAMESPACE_END
