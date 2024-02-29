//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_UNORDERED_STRING_MAP_HPP
#define FITORIA_CORE_UNORDERED_STRING_MAP_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/detail/unordered_string_map.hpp>

#include <unordered_map>

FITORIA_NAMESPACE_BEGIN

template <typename T>
using unordered_string_map
    = std::unordered_map<std::string, T, detail::string_hash, std::equal_to<>>;

template <typename T>
using unordered_string_multimap = std::
    unordered_multimap<std::string, T, detail::string_hash, std::equal_to<>>;

FITORIA_NAMESPACE_END

#endif
