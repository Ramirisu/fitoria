//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_HTTP_DETAIL_HTTP_HPP
#define FITORIA_WEB_HTTP_DETAIL_HTTP_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web::http::detail {

using boost::beast::http::async_read;
using boost::beast::http::async_read_header;
using boost::beast::http::async_write;
using boost::beast::http::async_write_header;
using boost::beast::http::empty_body;
using boost::beast::http::fields;
using boost::beast::http::request;
using boost::beast::http::request_parser;
using boost::beast::http::request_serializer;
using boost::beast::http::response;
using boost::beast::http::string_body;
using boost::beast::http::to_status_class;
using boost::beast::http::vector_body;
}

FITORIA_NAMESPACE_END

#endif
