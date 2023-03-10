//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_CORE_CONFIG_HPP
#define FITORIA_CORE_CONFIG_HPP

#include <cassert>

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
#else
#define FITORIA_NO_EXCEPTIONS 1
#define FITORIA_TRY
#define FITORIA_CATCH(...) if (true) { } else
#define FITORIA_RETHROW() ((void)0)
#endif
// clang-format on

#if defined(_MSC_VER) && !defined(__clang__)
#define FITORIA_CXX_COMPILER_MSVC
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define FITORIA_CXX_COMPILER_GCC
#endif

#if defined(__cpp_lib_format) && __has_include(<format>)
#define FITORIA_HAS_STD_FORMAT
#elif __has_include(<fmt/core.h>)
#define FITORIA_HAS_FMT
#else
#error "requires formatting library"
#endif

#endif
