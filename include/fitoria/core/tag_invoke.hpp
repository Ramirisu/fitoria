//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_CORE_TAG_INVOKE_HPP
#define FITORIA_CORE_TAG_INVOKE_HPP

#include <fitoria/core/config.hpp>

#include <type_traits>

FITORIA_NAMESPACE_BEGIN

namespace detail {
void tag_invoke();

// clang-format off

template <typename Tag, typename... Args>
concept tag_invocable = requires(Tag tag, Args&&... args) {
  tag_invoke(static_cast<Tag&&>(tag), static_cast<Args&&>(args)...);
};

template <typename Tag, typename... Args>
concept nothrow_tag_invocable = tag_invocable<Tag, Args...>
  && requires(Tag tag, Args&&... args) {
    { tag_invoke(static_cast<Tag&&>(tag), static_cast<Args&&>(args)...) } noexcept;
  };

// clang-format on

template <typename Tag, typename... Args>
using tag_invoke_result_t
    = decltype(tag_invoke(std::declval<Tag>(), std::declval<Args>()...));

template <typename Tag, typename... Args>
struct tag_invoke_result { };

template <typename Tag, typename... Args>
  requires tag_invocable<Tag, Args...>
struct tag_invoke_result<Tag, Args...> {
  using type = tag_invoke_result_t<Tag, Args...>;
};

struct tag_invoke_t {
  template <typename Tag, typename... Args>
    requires(tag_invocable<Tag, Args...>)
  constexpr auto operator()(Tag tag, Args&&... args) const
      noexcept(nothrow_tag_invocable<Tag, Args...>)
          -> tag_invoke_result_t<Tag, Args...>
  {
    return tag_invoke(static_cast<Tag&&>(tag), static_cast<Args&&>(args)...);
  }
};

}

using detail::tag_invoke_t;

inline constexpr tag_invoke_t tag_invoke {};

using detail::tag_invocable;

using detail::nothrow_tag_invocable;

using detail::tag_invoke_result;

using detail::tag_invoke_result_t;

FITORIA_NAMESPACE_END

#endif
