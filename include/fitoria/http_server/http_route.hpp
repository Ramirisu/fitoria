//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/url.hpp>

FITORIA_NAMESPACE_BEGIN

class http_route {
public:
  explicit http_route(std::string_view path,
                      std::string_view params_as_query_string)
  {
    url_.set_path(path);
    url_.set_encoded_query(params_as_query_string);
  }

  /// @brief get the routing path that is configured for the handler
  /// @return "/api/v1/users/{user}"
  std::string path() const noexcept
  {
    return url_.path();
  }

  urls::segments_view segments() noexcept
  {
    return url_.segments();
  }

  urls::params_encoded_view encoded_params() noexcept
  {
    return url_.encoded_params();
  }

  urls::params_view params() noexcept
  {
    return url_.params();
  }

private:
  urls::url url_;
};

FITORIA_NAMESPACE_END
