//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_FROM_HTTP_REQUEST_HPP
#define FITORIA_WEB_FROM_HTTP_REQUEST_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/error.hpp>
#include <fitoria/core/expected.hpp>
#include <fitoria/core/lazy.hpp>
#include <fitoria/core/tag_invoke.hpp>

#include <fitoria/web/http_request.hpp>
#include <fitoria/web/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename R>
struct from_http_request_t {
  constexpr auto operator()(http_request& req) const noexcept(
      is_nothrow_tag_invocable_v<from_http_request_t<R>, http_request&>)
    requires is_tag_invocable_v<from_http_request_t<R>, http_request&>
  {
    static_assert(std::is_same_v<
                  tag_invoke_result_t<from_http_request_t<R>, http_request&>,
                  lazy<expected<R, error_code>>>);
    return tag_invoke(from_http_request_t<R> {}, req);
  }

  friend auto tag_invoke(from_http_request_t<R>, http_request& req)
      -> lazy<expected<R, error_code>>
    requires(uncvref_same_as<R, http_request>)
  {
    co_return req;
  }

  friend auto tag_invoke(from_http_request_t<R>, http_request& req)
      -> lazy<expected<R, error_code>>
    requires(uncvref_same_as<R, route_params>)
  {
    co_return req.params();
  }

  friend auto tag_invoke(from_http_request_t<R>, http_request& req)
      -> lazy<expected<R, error_code>>
    requires(uncvref_same_as<R, query_map>)
  {
    co_return req.query();
  }

  friend auto tag_invoke(from_http_request_t<R>, http_request& req)
      -> lazy<expected<R, error_code>>
    requires(uncvref_same_as<R, http_fields>)
  {
    co_return req.fields();
  }

  friend auto tag_invoke(from_http_request_t<R>, http_request& req)
      -> lazy<expected<R, error_code>>
    requires(std::same_as<R, std::vector<std::byte>>)
  {
    co_return co_await req.body().read_all();
  }

  friend auto tag_invoke(from_http_request_t<R>, http_request& req)
      -> lazy<expected<R, error_code>>
    requires(std::same_as<R, std::string>)
  {
    auto chunk = co_await req.body().read_next();
    if (!chunk) {
      co_return unexpected { chunk.error() };
    }

    std::string data;
    while (chunk) {
      data.append(reinterpret_cast<const char*>(chunk->data()), chunk->size());
      chunk = co_await req.body().read_next();
    }

    if (!chunk && chunk.error() != http::error::end_of_stream) {
      co_return unexpected { chunk.error() };
    }

    co_return data;
  }
};

}

FITORIA_NAMESPACE_END

#endif
