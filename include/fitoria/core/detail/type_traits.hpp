//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <type_traits>
#include <variant>

FITORIA_NAMESPACE_BEGIN

namespace detail {

template <typename... Ts>
auto as_variant(const std::variant<Ts...>& v)
{
  return v;
}

}

FITORIA_NAMESPACE_END
