//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_TEST_MEMFN_HPP
#define FITORIA_TEST_MEMFN_HPP

#include <fitoria/core/config.hpp>

#include <utility>

FITORIA_NAMESPACE_BEGIN

namespace test {

template <typename T>
using memop_mem_of = decltype(std::declval<T>().operator->());

template <typename T>
using memop_deref = decltype(std::declval<T>().operator*());

template <typename T>
using memfn_value = decltype(std::declval<T>().value());

template <typename T>
using memfn_error = decltype(std::declval<T>().error());

}

FITORIA_NAMESPACE_END

#endif
