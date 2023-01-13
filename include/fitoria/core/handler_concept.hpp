//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/coroutine_concept.hpp>

FITORIA_NAMESPACE_BEGIN

template <typename HandlerTrait>
using middleware_input_param_t =
    typename HandlerTrait::middleware_input_param_t;

template <typename HandlerTrait>
using middleware_t = typename HandlerTrait::middleware_t;

template <typename HandlerTrait>
inline constexpr bool middleware_result_awaitable
    = awaitable<typename middleware_t<HandlerTrait>::result_type>;

template <typename HandlerTrait>
using middleware_result_t = typename middleware_t<HandlerTrait>::result_type;

template <typename HandlerTrait>
using middleware_compare_t = typename HandlerTrait::middleware_compare_t;

template <typename HandlerTrait>
using handler_input_param_t = typename HandlerTrait::handler_input_param_t;

template <typename HandlerTrait>
using handler_t = typename HandlerTrait::handler_t;

template <typename HandlerTrait>
inline constexpr bool handler_result_awaitable
    = awaitable<typename handler_t<HandlerTrait>::result_type>;

template <typename HandlerTrait>
using handler_result_t = typename handler_t<HandlerTrait>::result_type;

template <typename HandlerTrait>
using handler_compare_t = typename HandlerTrait::handler_compare_t;

FITORIA_NAMESPACE_END
