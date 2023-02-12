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

// Workaround: Clang 15 bug?
// using concept to define tag_invocable results `constraints not satisfied`
// errors
template <typename Void, typename Tag, typename... Args>
struct is_tag_invocable_impl : std::false_type { };

template <typename Tag, typename... Args>
struct is_tag_invocable_impl<
    std::void_t<decltype(tag_invoke(std::declval<Tag>(),
                                    std::declval<Args>()...))>,
    Tag,
    Args...> : std::true_type { };

template <typename Tag, typename... Args>
struct is_tag_invocable : is_tag_invocable_impl<void, Tag, Args...> { };

template <typename Tag, typename... Args>
inline constexpr bool is_tag_invocable_v
    = is_tag_invocable<Tag, Args...>::value;

template <bool, typename Tag, typename... Args>
struct is_nothrow_tag_invocable_impl : std::false_type { };

template <typename Tag, typename... Args>
struct is_nothrow_tag_invocable_impl<true, Tag, Args...>
    : std::bool_constant<noexcept(
          tag_invoke(std::declval<Tag>(), std::declval<Args>()...))> { };

template <typename Tag, typename... Args>
struct is_nothrow_tag_invocable
    : is_nothrow_tag_invocable_impl<is_tag_invocable_v<Tag, Args...>,
                                    Tag,
                                    Args...> { };

template <typename Tag, typename... Args>
inline constexpr bool is_nothrow_tag_invocable_v
    = is_nothrow_tag_invocable<Tag, Args...>::value;

template <typename Tag, typename... Args>
struct tag_invoke_result { };

template <typename Tag, typename... Args>
  requires is_tag_invocable_v<Tag, Args...>
struct tag_invoke_result<Tag, Args...> {
  using type
      = decltype(tag_invoke(std::declval<Tag>(), std::declval<Args>()...));
};

template <typename Tag, typename... Args>
using tag_invoke_result_t = typename tag_invoke_result<Tag, Args...>::type;

struct tag_invoke_t {
  template <typename Tag, typename... Args>
    requires(is_tag_invocable_v<Tag, Args...>)
  constexpr auto operator()(Tag tag, Args&&... args) const
      -> tag_invoke_result_t<Tag, Args...>
  {
    return tag_invoke(static_cast<Tag&&>(tag), static_cast<Args&&>(args)...);
  }
};

}

using detail::tag_invoke_t;

inline constexpr tag_invoke_t tag_invoke {};

using detail::is_tag_invocable_v;

using detail::is_nothrow_tag_invocable_v;

using detail::tag_invoke_result;

using detail::tag_invoke_result_t;

FITORIA_NAMESPACE_END

#endif
