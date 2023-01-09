//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/log/logger.hpp>
#include <fitoria/log/writer.hpp>

#include <iostream>
#include <mutex>

FITORIA_NAMESPACE_BEGIN

namespace log {

class stdout_writer : public writer {
public:
  ~stdout_writer() override = default;

  void write(std::string msg) override
  {
    [[maybe_unused]] auto lock = std::scoped_lock(mutex_);
    std::cout << msg;
  }

private:
  std::mutex mutex_;
};

std::shared_ptr<logger> stdout_logger()
{
  return std::make_shared<logger>(std::make_shared<stdout_writer>());
}

}

FITORIA_NAMESPACE_END
