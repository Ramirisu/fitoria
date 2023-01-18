//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_CORE_COROUTINE_CONCEPT_HPP
#define FITORIA_CORE_COROUTINE_CONCEPT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/detail/coroutine_concept.hpp>

FITORIA_NAMESPACE_BEGIN

template <typename T>
concept awaitable = detail::member_co_awaitable<T>
    || detail::free_co_awaitable<T> || detail::awaiter<T>;

template <typename T>
using awaiter_type_t = decltype(detail::get_awaiter(std::declval<T>()));

template <typename T>
using await_result_t
    = decltype(std::declval<awaiter_type_t<T>&>().await_resume());

FITORIA_NAMESPACE_END

#endif
