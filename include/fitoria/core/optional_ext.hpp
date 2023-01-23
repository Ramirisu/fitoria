//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_CORE_OPTIONAL_EXT_HPP
#define FITORIA_CORE_OPTIONAL_EXT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/optional.hpp>

FITORIA_NAMESPACE_BEGIN

template <typename E>
constexpr optional<void> value_to_optional(const expected<void, E>& exp)
{
  if (exp) {
    return optional<void>(std::in_place);
  } else {
    return optional<void>();
  }
}
template <typename E>
constexpr optional<void> value_to_optional(expected<void, E>&& exp)
{
  if (exp) {
    return optional<void>(std::in_place);
  } else {
    return optional<void>();
  }
}

template <typename E>
constexpr optional<E> error_to_optional(const expected<void, E>& exp)
{
  if (exp) {
    return optional<E>();
  } else {
    return optional<E>(exp.error());
  }
}

template <typename E>
constexpr optional<E> error_to_optional(expected<void, E>&& exp)
{
  if (exp) {
    return optional<E>();
  } else {
    return optional<E>(std::move(exp.error()));
  }
}

template <typename T, typename E>
constexpr optional<T> value_to_optional(const expected<T, E>& exp)
{
  if (exp) {
    return optional<T>(exp.value());
  } else {
    return optional<T>();
  }
}

template <typename T, typename E>
constexpr optional<T> value_to_optional(expected<T, E>&& exp)
{
  if (exp) {
    return optional<T>(std::move(exp.value()));
  } else {
    return optional<T>();
  }
}

template <typename T, typename E>
constexpr optional<E> error_to_optional(const expected<T, E>& exp)
{
  if (exp) {
    return optional<E>();
  } else {
    return optional<E>(exp.error());
  }
}

template <typename T, typename E>
constexpr optional<E> error_to_optional(expected<T, E>&& exp)
{
  if (exp) {
    return optional<E>();
  } else {
    return optional<E>(std::move(exp.error()));
  }
}

FITORIA_NAMESPACE_END

#endif
