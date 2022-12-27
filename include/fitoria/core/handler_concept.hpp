//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/coroutine_concept.hpp>

FITORIA_NAMESPACE_BEGIN

template <typename HandlerTrait>
using handler_t = typename HandlerTrait::handler_t;

template <typename HandlerTrait>
using handlers_t = typename HandlerTrait::handlers_t;

template <typename HandlerTrait>
using handler_result_t = typename HandlerTrait::handler_result_t;

template <typename HandlerTrait>
inline constexpr bool handler_result_awaitable
    = awaitable<handler_result_t<HandlerTrait>>;

template <typename HandlerTrait>
using handler_compare_t = typename HandlerTrait::handler_compare_t;

FITORIA_NAMESPACE_END
