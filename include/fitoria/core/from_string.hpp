//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_FROM_STRING_HPP
#define FITORIA_CORE_FROM_STRING_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/detail/from_string_impl.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/tag_invoke.hpp>
#include <fitoria/core/utility.hpp>

#include <string_view>

FITORIA_NAMESPACE_BEGIN

namespace from_string_ns {
template <typename R>
struct from_string_t {
  auto operator()(std::string_view s) const
      noexcept(is_nothrow_tag_invocable_v<from_string_t<R>, std::string_view>)
    requires is_tag_invocable_v<from_string_t<R>, std::string_view>
  {
    static_assert(
        std::same_as<tag_invoke_result_t<from_string_t<R>, std::string_view>,
                     expected<R, std::error_code>>);
    return tag_invoke(*this, s);
  }

  friend auto tag_invoke(from_string_t<R>,
                         std::string_view s) -> expected<R, std::error_code>
    requires(std::same_as<R, std::string_view>)
  {
    return s;
  }

  friend auto tag_invoke(from_string_t<R>,
                         std::string_view s) -> expected<R, std::error_code>
    requires(std::same_as<R, std::string>)
  {
    return std::string(s);
  }

  friend auto tag_invoke(from_string_t<R>,
                         std::string_view s) -> expected<R, std::error_code>
    requires(!std::same_as<R, bool> && std::is_integral_v<R>)
  {
    return detail::from_string_impl<R>(s);
  }

  friend auto tag_invoke(from_string_t<R>,
                         std::string_view s) -> expected<R, std::error_code>
    requires(!std::same_as<R, bool> && std::is_floating_point_v<R>)
  {
    return detail::from_string_impl<R>(s);
  }

  friend auto tag_invoke(from_string_t<R>,
                         std::string_view s) -> expected<R, std::error_code>
    requires std::same_as<R, bool>
  {
    if (iequals(s, "1") || iequals(s, "t") || iequals(s, "true")
        || iequals(s, "y") || iequals(s, "yes")) {
      return true;
    }
    if (iequals(s, "0") || iequals(s, "f") || iequals(s, "false")
        || iequals(s, "n") || iequals(s, "no")) {
      return false;
    }

    return unexpected { make_error_code(std::errc::invalid_argument) };
  }
};

}

using from_string_ns::from_string_t;
template <typename R>
inline constexpr from_string_t<R> from_string {};

FITORIA_NAMESPACE_END

#endif
