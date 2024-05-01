//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_ASYNC_FORMATTABLE_WRITER_HPP
#define FITORIA_LOG_ASYNC_FORMATTABLE_WRITER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/log/async_writer.hpp>
#include <fitoria/log/formatter.hpp>

FITORIA_NAMESPACE_BEGIN

namespace log {

class async_formattable_writer : public async_writer {
public:
  async_formattable_writer() = default;

  async_formattable_writer(formatter fmter)
      : fmter_(std::move(fmter))
  {
  }

  ~async_formattable_writer() override = default;

  void set_formatter(formatter fmter)
  {
    fmter_ = fmter;
  }

protected:
  formatter fmter_;
};

}

FITORIA_NAMESPACE_END

#endif
