//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <boost/beast/http.hpp>

FITORIA_NAMESPACE_BEGIN

namespace http = boost::beast::http;

using verb = http::verb;
using field = http::field;
using status = http::status;

FITORIA_NAMESPACE_END
