//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ROUTE_PARAMS_HPP
#define FITORIA_WEB_ROUTE_PARAMS_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class route_params : public query_map {
public:
  route_params() = default;

  route_params(query_map params, std::string path)
      : query_map(std::move(params))
      , path_(std::move(path))
  {
  }

  const std::string& path() const noexcept
  {
    return path_;
  }

private:
  std::string path_;
};

}

FITORIA_NAMESPACE_END

#endif
