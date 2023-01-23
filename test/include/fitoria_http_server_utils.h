//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_FITORIA_HTTP_SERVER_UTILS_HPP
#define FITORIA_FITORIA_HTTP_SERVER_UTILS_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/url.hpp>

FITORIA_NAMESPACE_BEGIN

namespace http_server_utils {

inline const auto server_start_wait_time = std::chrono::milliseconds(200);
inline const char* server_ip = "127.0.0.1";
inline const char* localhost = "localhost";
inline std::uint16_t generate_port()
{
  static std::uint16_t port = 43210;
  return ++port;
}

inline std::string
to_local_url(urls::scheme scheme, std::uint16_t port, std::string_view path)
{
  return fmt::vformat("{}://{}:{}{}",
                      fmt::make_format_args(std::string_view(to_string(scheme)),
                                            localhost, port, path));
}

}

FITORIA_NAMESPACE_END

#endif
