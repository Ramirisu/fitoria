//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <gul/expected.hpp>
#include <gul/optional.hpp>

#include <unordered_map>

FITORIA_NAMESPACE_BEGIN

using gul::in_place;

using gul::expected;
using gul::unexpect;
using gul::unexpected;

using gul::nullopt;
using gul::optional;

using gul::function_traits;
using gul::is_specialization_of;

namespace detail {

struct string_hash {
  using is_transparent = void;

  size_t operator()(const char* s) const
  {
    return std::hash<std::string_view> {}(s);
  }

  size_t operator()(std::string_view sv) const
  {
    return std::hash<std::string_view> {}(sv);
  }

  size_t operator()(const std::string& s) const
  {
    return std::hash<std::string> {}(s);
  }
};

}

template <typename T>
using unordered_string_map
    = std::unordered_map<std::string, T, detail::string_hash, std::equal_to<>>;

template <typename T>
using unordered_string_multimap = std::
    unordered_multimap<std::string, T, detail::string_hash, std::equal_to<>>;

FITORIA_NAMESPACE_END
