//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_FORMAT_HPP
#define FITORIA_CORE_FORMAT_HPP

#include <fitoria/core/config.hpp>

#if defined(FITORIA_HAS_STD_FORMAT)
#include <format>

FITORIA_NAMESPACE_BEGIN

namespace fmt {
using std::format;
using std::format_string;
using std::format_to;
using std::formatter;
}

FITORIA_NAMESPACE_END

#elif defined(FITORIA_HAS_FMT)
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>

FITORIA_NAMESPACE_BEGIN

namespace fmt = fmt;

FITORIA_NAMESPACE_END

#endif

#endif
