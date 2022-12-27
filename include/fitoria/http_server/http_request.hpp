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
#include <fitoria/core/utility.hpp>

#include <fitoria/http_server/http_header.hpp>

FITORIA_NAMESPACE_BEGIN

class http_request {
  using native_type = http::request<http::string_body>;

public:
  explicit http_request(native_type& native,
                        std::string path,
                        unordered_string_map<std::string> params)
      : native_(native)
      , path_(std::move(path))
      , params_(std::move(params))
  {
    for (auto it = native.begin(); it != native.end(); ++it) {
      header_.set(it->name_string(), it->value());
    }
  }

  verb method() const noexcept
  {
    return native_.method();
  }

  std::string& path() noexcept
  {
    return path_;
  }

  const std::string& path() const noexcept
  {
    return path_;
  }

  unordered_string_map<std::string>& params() noexcept
  {
    return params_;
  }

  const unordered_string_map<std::string>& params() const noexcept
  {
    return params_;
  }

  http_header& headers() noexcept
  {
    return header_;
  }

  const http_header& headers() const noexcept
  {
    return header_;
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
  std::string path_;
  unordered_string_map<std::string> params_;
  http_header header_;
};

FITORIA_NAMESPACE_END
