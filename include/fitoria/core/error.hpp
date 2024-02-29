//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_ERROR_HPP
#define FITORIA_CORE_ERROR_HPP

#include <fitoria/core/config.hpp>

#include <system_error>

FITORIA_NAMESPACE_BEGIN

using error_code = std::error_code;

using system_error = std::system_error;

FITORIA_NAMESPACE_END

#endif
