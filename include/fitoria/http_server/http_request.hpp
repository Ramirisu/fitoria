//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>
#include <fitoria/core/url.hpp>

FITORIA_NAMESPACE_BEGIN

class http_request {
  using native_type = http::request<http::string_body>;

public:
  explicit http_request(native_type& native, urls::url_view url)
      : native_(native)
      , url_(std::move(url))
  {
  }

  methods method() const noexcept
  {
    return native_.method();
  }

  urls::pct_string_view encoded_path() const noexcept
  {
    return url_.encoded_path();
  }

  std::string path() const noexcept
  {
    return url_.path();
  }

  urls::pct_string_view encoded_query() const noexcept
  {
    return url_.encoded_query();
  }

  std::string query() const noexcept
  {
    return url_.query();
  }

  urls::params_encoded_view encoded_params() const noexcept
  {
    return url_.encoded_params();
  }

  urls::params_view params() const noexcept
  {
    return url_.params();
  }

  std::string& body() noexcept
  {
    return native_.body();
  }

  const std::string& body() const noexcept
  {
    return native_.body();
  }

private:
  native_type& native_;
  urls::url_view url_;
};

FITORIA_NAMESPACE_END
