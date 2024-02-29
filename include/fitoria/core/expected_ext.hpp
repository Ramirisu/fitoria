//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_EXPECTED_EXT_HPP
#define FITORIA_CORE_EXPECTED_EXT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/optional.hpp>

FITORIA_NAMESPACE_BEGIN

template <typename E>
constexpr expected<void, std::remove_cvref_t<E>>
to_expected(const optional<void>& opt, E&& default_error)
{
  if (opt) {
    return expected<void, std::remove_cvref_t<E>>();
  } else {
    return expected<void, std::remove_cvref_t<E>>(
        unexpect, std::forward<E>(default_error));
  }
}

template <typename E>
constexpr expected<void, std::remove_cvref_t<E>>
to_expected(optional<void>&& opt, E&& default_error)
{
  if (opt) {
    return expected<void, std::remove_cvref_t<E>>();
  } else {
    return expected<void, std::remove_cvref_t<E>>(
        unexpect, std::forward<E>(default_error));
  }
}

template <typename T, typename E>
constexpr expected<T, std::remove_cvref_t<E>>
to_expected(const optional<T>& opt, E&& default_error)
{
  if (opt) {
    return expected<T, std::remove_cvref_t<E>>(opt.value());
  } else {
    return expected<T, std::remove_cvref_t<E>>(unexpect,
                                               std::forward<E>(default_error));
  }
}

template <typename T, typename E>
constexpr expected<T, std::remove_cvref_t<E>> to_expected(optional<T>&& opt,
                                                          E&& default_error)
{
  if (opt) {
    return expected<T, std::remove_cvref_t<E>>(std::move(opt.value()));
  } else {
    return expected<T, std::remove_cvref_t<E>>(unexpect,
                                               std::forward<E>(default_error));
  }
}

FITORIA_NAMESPACE_END

#endif
