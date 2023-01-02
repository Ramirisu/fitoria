//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/http_server/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

class http_route : public query_map {
public:
  http_route(query_map params, std::string path)
      : query_map(std::move(params))
      , path_(std::move(path))
  {
  }

  /// @brief get the route configured for the handler
  /// @return "/api/v1/users/{user}"
  std::string& path() noexcept
  {
    return path_;
  }

  const std::string& path() const noexcept
  {
    return path_;
  }

private:
  std::string path_;
};

FITORIA_NAMESPACE_END
