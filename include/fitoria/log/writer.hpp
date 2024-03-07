//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_WRITER_HPP
#define FITORIA_LOG_WRITER_HPP

#include <fitoria/core/config.hpp>

#include <string>

FITORIA_NAMESPACE_BEGIN

namespace log {

class writer {
public:
  virtual ~writer() = default;

  virtual void write(std::string msg) = 0;
};

}

FITORIA_NAMESPACE_END

#endif
