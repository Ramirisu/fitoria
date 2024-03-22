//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_CONFIG_HPP
#define FITORIA_CORE_CONFIG_HPP

#include <cassert>
#include <version>

#define FITORIA_NAMESPACE_BEGIN                                                \
  namespace fitoria {                                                          \
  inline namespace v0 {
#define FITORIA_NAMESPACE_END                                                  \
  }                                                                            \
  }

#define FITORIA_NAMESPACE fitoria::v0

#define FITORIA_ASSERT(expr) assert(expr)

// clang-format off
#if __cpp_exceptions >= 199711
#define FITORIA_NO_EXCEPTIONS 0
#define FITORIA_TRY try
#define FITORIA_CATCH(...) catch (__VA_ARGS__)
#define FITORIA_RETHROW() throw
#define FITORIA_THROW(ex) throw ex
#else
#include <exception>
#define FITORIA_NO_EXCEPTIONS 1
#define FITORIA_TRY
#define FITORIA_CATCH(...) if (true) { } else
#define FITORIA_RETHROW() ((void)0)
#define FITORIA_THROW(ex) std::terminate()
#endif
// clang-format on

#if defined(_MSC_VER) && !defined(__clang__)
#define FITORIA_CXX_COMPILER_MSVC
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define FITORIA_CXX_COMPILER_GCC
#endif

#if defined(_WIN32)
#define FITORIA_TARGET_WINDOWS
#elif defined(__APPLE__)
#define FITORIA_TARGET_MACOS
#else
#define FITORIA_TARGET_LINUX
#endif

#if !__has_include(<fmt/format.h>)
#error "requires fmtlib"
#endif

#if defined(__cpp_lib_source_location)
#define FITORIA_HAS_STD_SOURCE_LOCATION
#endif

#if __has_include(<boost/pfr.hpp>)
#define FITORIA_HAS_BOOST_PFR
#endif

#endif
