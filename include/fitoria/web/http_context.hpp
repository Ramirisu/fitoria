//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_HTTP_CONTEXT_HPP
#define FITORIA_WEB_HTTP_CONTEXT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

#include <fitoria/web/http_request.hpp>
#include <fitoria/web/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class http_context {
public:
  http_context(http_request& request)
      : request_(request)
  {
  }

  http_request& request() noexcept
  {
    return request_;
  }

  const http_request& request() const noexcept
  {
    return request_;
  }

  operator http_request&() noexcept
  {
    return request_;
  }

  operator const http_request&() const noexcept
  {
    return request_;
  }

private:
  http_request& request_;
};

}

FITORIA_NAMESPACE_END

#endif
