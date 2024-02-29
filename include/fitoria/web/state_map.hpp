//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_STATE_MAP_HPP
#define FITORIA_WEB_STATE_MAP_HPP

#include <fitoria/core/config.hpp>

#include <any>
#include <typeindex>
#include <unordered_map>

FITORIA_NAMESPACE_BEGIN

namespace web {

using state_map = std::unordered_map<std::type_index, std::any>;

}

FITORIA_NAMESPACE_END

#endif
