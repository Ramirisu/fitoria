//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#if defined(FITORIA_HAS_STD_FORMAT)
#include <format>

FITORIA_NAMESPACE_BEGIN

namespace fmt {
using std::format;
using std::format_to;
using std::formatter;
using std::make_format_args;
using std::vformat;
}

FITORIA_NAMESPACE_END

#elif defined(FITORIA_HAS_FMT)
#include <fmt/chrono.h>
#include <fmt/format.h>

FITORIA_NAMESPACE_BEGIN

namespace fmt = fmt;

FITORIA_NAMESPACE_END

#endif
