//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/detail/utility.hpp>

#include <gul/expected.hpp>

#include <unordered_map>

FITORIA_NAMESPACE_BEGIN

using gul::in_place;

using gul::expected;
using gul::unexpect;
using gul::unexpected;

using gul::function_traits;

template <typename T>
using unordered_string_map
    = std::unordered_map<std::string, T, detail::string_hash, std::equal_to<>>;

template <typename T>
using unordered_string_multimap = std::
    unordered_multimap<std::string, T, detail::string_hash, std::equal_to<>>;

FITORIA_NAMESPACE_END
