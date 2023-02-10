//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_FROM_HTTP_REQUEST_HPP
#define FITORIA_WEB_FROM_HTTP_REQUEST_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/tag_invoke.hpp>

#include <fitoria/web/http_request.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename R>
struct from_http_request_t {
  constexpr auto operator()(http_request& req) const
      noexcept(nothrow_tag_invocable<from_http_request_t<R>, http_request&>)
    requires tag_invocable<from_http_request_t<R>, http_request&>
  {
    return FITORIA_NAMESPACE::tag_invoke_f(from_http_request_t<R> {}, req);
  }

  friend R tag_invoke(from_http_request_t<R>, http_request& req)
    requires(uncvref_same_as<R, http_request>)
  {
    return req;
  }

  friend R tag_invoke(from_http_request_t<R>, http_request& req)
    requires(uncvref_same_as<R, route_params>)
  {
    return req.params();
  }

  friend R tag_invoke(from_http_request_t<R>, http_request& req)
    requires(uncvref_same_as<R, query_map>)
  {
    return req.query();
  }

  friend R tag_invoke(from_http_request_t<R>, http_request& req)
    requires(uncvref_same_as<R, http_fields>)
  {
    return req.fields();
  }

  friend R tag_invoke(from_http_request_t<R>, http_request& req)
    requires(uncvref_same_as<R, std::string>)
  {
    return req.body();
  }
};

}

FITORIA_NAMESPACE_END

#endif
