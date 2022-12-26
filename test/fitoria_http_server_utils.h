//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

FITORIA_NAMESPACE_BEGIN

namespace http_server_utils {

inline const auto server_start_wait_time = std::chrono::milliseconds(500);
inline const char* server_ip = "127.0.0.1";
inline const char* localhost = "localhost";
inline std::uint16_t generate_port()
{
  static std::uint16_t port = 50000;
  return ++port;
}

}

FITORIA_NAMESPACE_END
